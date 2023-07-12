#include "UvcGrabber.hpp"
#include <time.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/videodev2.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "folder-functions.hpp"
#include <experimental/filesystem>
#include <exception>

namespace
{
    std::string getTime();
}


UvcGrabber::UvcGrabber():
    _cameraDeviceName("/dev/video0")
{}

UvcGrabber::~UvcGrabber()
{}

void UvcGrabber::setDevice(const std::string& deviceName)
{
    _cameraDeviceName = deviceName;
}
bool UvcGrabber::GrabFrames(int frames, const std::string& fullFolderPath, int frameWidth /*= 640 */, int frameHeight /*= 480 */)
{
	int fd = open(_cameraDeviceName.c_str(), O_RDWR | O_NONBLOCK);
	if (fd == -1) 
	{
        std::cerr << "Failed to open device\n";
        return false;
    }
    struct v4l2_buffer buffers[4];
    void* mem[4];
    unsigned int n_buffers;
    try {
        std::string errorMsg = "Failed to query capability\n";

        struct v4l2_capability cap;
        struct v4l2_format fmt;
        struct v4l2_requestbuffers req;
        enum v4l2_buf_type type;

        Ioctl(fd, VIDIOC_QUERYCAP, &cap, errorMsg);

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
        {
            close(fd);
            throw std::runtime_error("Device does not support video capture");
        }

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = frameWidth;
        fmt.fmt.pix.height = frameHeight;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        errorMsg = "Failed to set format\n";
        Ioctl(fd, VIDIOC_S_FMT, &fmt, errorMsg);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        errorMsg = "Failed to request buffers\n";
        Ioctl(fd, VIDIOC_REQBUFS, &req, errorMsg);
        n_buffers = req.count;

        for (unsigned int i = 0; i < n_buffers; ++i) 
        {
            buffers[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffers[i].memory = V4L2_MEMORY_MMAP;
            buffers[i].index = i;

            Ioctl(fd, VIDIOC_QUERYBUF, &buffers[i], std::string("Failed to query buffer"));

            mem[i] = mmap(NULL, buffers[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffers[i].m.offset);
            if (mem[i] == MAP_FAILED) 
            {
                close(fd);
                throw std::runtime_error("Failed to map buffer");
            }

            Ioctl(fd, VIDIOC_QBUF, &buffers[i], std::string("Failed to queue buffer"));
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        errorMsg = "Failed to start streaming\n";
        Ioctl(fd, VIDIOC_STREAMON, &type, errorMsg);
        int count = 1;
        while (count <= frames)
        {
            sleep(1);
            struct v4l2_buffer buf;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            Ioctl(fd, VIDIOC_DQBUF, &buf, std::string("Failed to dequeue buffer"));
            char filename[256];
            snprintf(filename, sizeof(filename), "%s/frame%d.jpg", fullFolderPath.c_str(), count);
            FILE* file = fopen(filename, "wb");
            if (file == NULL)
            {
                close(fd);
                throw std::runtime_error("Failed to open file");
            } 

            _framesTimeVec.push_back(getTime());

            fwrite(mem[buf.index], buf.bytesused, 1, file);
            fclose(file);

            Ioctl(fd, VIDIOC_QBUF, &buf, std::string("Failed to queue buffer"));
        	count++;
        }

        Ioctl(fd, VIDIOC_STREAMOFF, &type, std::string("Failed to stop streaming"));
        freeResources(fd, n_buffers, mem, buffers);
        return true;
    } catch (std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        return false;
    }
}

bool UvcGrabber::AddFrameTimeTag(std::string& fullFolderPath, int coordX /* = 10*/, int coordY /* = 30 */, std::string ext /*  = ".jpg*/)
{
    namespace fs = std::experimental::filesystem;

    if (folder_funcs::isDirectoryEmpty(fullFolderPath))
    {
        std::cerr << "Директория кадров пуста!\n";
        return false;
    }
    int frameTimeIndex = 0;
    for (const auto& entry : fs::directory_iterator(fullFolderPath))
    {
        std::string filename = entry.path().filename().string();
        if (filename.length() < 4 || filename.substr(filename.length() - 4) != std::string(ext)) 
            continue;
        if (fullFolderPath.back() == '/')
            fullFolderPath.pop_back();
        std::string filePath = fullFolderPath + "/" + filename;
        auto image = getOpenCVImage(filePath);

        putOpenCVText(coordX, coordY, image, _framesTimeVec[frameTimeIndex++]);
        writeImageOpenCV(filePath, image);
    }
    return true;
}

bool UvcGrabber::CaptureVideo(int duration, const std::string& fullFolderPath)
{
    using namespace cv;
    VideoCapture cap(_cameraDeviceName);
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video device in Video Capture function!\n";
        return false;
    }

    time_t now = time(NULL);
    struct tm* localTime = localtime(&now);
    std::stringstream ss;
    ss << fullFolderPath << "/video_" << localTime->tm_year + 1900 << "-"
       << std::setw(2) << std::setfill('0') << localTime->tm_mon + 1 << "-"
       << std::setw(2) << std::setfill('0') << localTime->tm_mday << "_"
       << std::setw(2) << std::setfill('0') << localTime->tm_hour << "-"
       << std::setw(2) << std::setfill('0') << localTime->tm_min << "-"
       << std::setw(2) << std::setfill('0') << localTime->tm_sec << ".avi";
    std::string videoPath = ss.str();


    VideoWriter writer(videoPath, VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, Size(640, 480));
    if (!writer.isOpened())
    {
        std::cerr << "Error creating video file!\n";
        return false;
    }

    time_t startTime = time(NULL);
    while (difftime(time(NULL), startTime) < duration)
    {
        Mat frame;
        cap >> frame;

        if (frame.empty())
        {
            std::cerr << "Error capturing frame!\n";
            break;
        }

        writer.write(frame);
        imshow("Захваченное видео", frame);
        if (waitKey(1) == 'q')
            break;

    }
    writer.release();
    cap.release();
    destroyAllWindows();
    return true;
}

void UvcGrabber::Ioctl(int fd, int request, void* arg, const std::string& errmsg)
{
    if (ioctl(fd, request, arg) == -1)
    {
        close(fd); 
        throw std::runtime_error(errmsg.c_str());
    }
}

void UvcGrabber::freeResources(int fd, int n, void** mem, struct v4l2_buffer* buffers)
{
    for (int i = 0; i < n; i++) {
        if (munmap(mem[i], buffers[i].length) < 0) {
            close(fd);
            throw std::runtime_error("Failed to unmap buffer");
        }
    }
    close(fd);
}

cv::Mat UvcGrabber::getOpenCVImage(const std::string& imagePath)
{   
    return cv::imread(imagePath);
}

void UvcGrabber::putOpenCVText(int coordX, int coordY, cv::Mat& image, const std::string& text)
{
    cv::putText(image, text, cv::Point(coordX, coordY), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(51, 102, 0), 1);
}

void UvcGrabber::writeImageOpenCV(const std::string& filePath, cv::Mat& image)
{
    cv::imwrite(filePath, image);
}

namespace
{
    std::string getTime()
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        auto timer = system_clock::to_time_t(now);

        std::tm bt = *std::localtime(&timer);

        std::stringstream ss;

        ss << std::setfill('0') << std::setw(4) << bt.tm_year + 1900
        << '.' << std::setfill('0') << std::setw(2) << bt.tm_mon + 1
        << '.' << std::setfill('0') << std::setw(2) << bt.tm_mday
        << "|"
        << std::setfill('0') << std::setw(2) << bt.tm_hour
        << ':' << std::setfill('0') << std::setw(2) << bt.tm_min
        << ':' << std::setfill('0') << std::setw(2) << bt.tm_sec
        << '.' << std::setfill('0') << std::setw(3) << ms.count(); 
        return ss.str();
    }
}
