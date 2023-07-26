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

namespace
{
    std::string getTime();
    std::string getVideoFolderName(const std::string& folderPath);
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
    ioManager.setFileDescriptor(fd);
    struct v4l2_buffer buffers[4];
    void* mem[4];
    unsigned int n_buffers;
    try {
        struct v4l2_capability cap;
        struct v4l2_format fmt;
        struct v4l2_requestbuffers req;
        enum v4l2_buf_type type;

        ioManager.queryCap(&cap);

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
        {
            close(fd);
            throw std::runtime_error("Device does not support video capture");
        }

        setFrameFormat(frameWidth, frameHeight, &fmt);
        ioManager.setFmt(&fmt);

        n_buffers = 4;
        setRequestedBuffers(n_buffers, &req);
        ioManager.requestBuffers(&req);

        for (unsigned int i = 0; i < n_buffers; ++i) 
        {
            setFrameBuffer(i, &buffers[i]);
            ioManager.queryBuffer(&buffers[i]);
            mem[i] = mmap(NULL, buffers[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffers[i].m.offset);
            if (mem[i] == MAP_FAILED) 
            {
                close(fd);
                throw std::runtime_error("Failed to map buffer");
            }
            ioManager.queueBuffer(&buffers[i]);
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioManager.startStreaming(&type);
        saveAllFrames(frames, fullFolderPath.c_str(), mem);

        ioManager.stopStreaming(&type);
        free_V4L_Resources(fd, n_buffers, mem, buffers);
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

    VideoWriter writer(getVideoFolderName(fullFolderPath), VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, Size(640, 480));
    if (!writer.isOpened())
    {
        std::cerr << "Error creating video file!\n";
        return false;
    }

    captureFrameByFrame(duration, cap, writer);
    free_OpenCV_Resources(writer, cap);
    return true;
}

void UvcGrabber::setFrameFormat(int width, int height, struct v4l2_format* fmt)
{
    fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt->fmt.pix.width = width;
    fmt->fmt.pix.height = height;
    fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
    fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;
}

void UvcGrabber::setRequestedBuffers(unsigned int req_buffers, struct v4l2_requestbuffers* req)
{
    req->count = req_buffers;
    req->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req->memory = V4L2_MEMORY_MMAP;
}

void UvcGrabber::setFrameBuffer(int index, struct v4l2_buffer* buffer)
{
    buffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer->memory = V4L2_MEMORY_MMAP;
    buffer->index = index;
}
 
void UvcGrabber::saveAllFrames(int frames, const char* folderPath, void** memory)
{
    int count = 1;
    while (count <= frames)
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(ioManager.getFileDescriptor(), &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(ioManager.getFileDescriptor() + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                continue;
            fprintf(stderr, "select error %d, %s\n", errno, strerror(errno));
            return;
        }
        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            return;
        } 

        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        ioManager.dequeueBuffer(&buf);
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/frame%d.jpg", folderPath, count);
        saveOneFrame(filename, memory, &buf);
        ioManager.queueBuffer(&buf);
        count++;
    }
}

void UvcGrabber::saveOneFrame(char* filename, void** memory, struct v4l2_buffer* buffer)
{
    FILE* file = fopen(filename, "wb");
    if (file == NULL)
    {
        close(ioManager.getFileDescriptor());
        throw std::runtime_error("Failed to open file");
    } 

    _framesTimeVec.push_back(getTime());

    fwrite(memory[buffer->index], buffer->bytesused, 1, file);
    fclose(file);
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

void UvcGrabber::captureFrameByFrame(int duration, cv::VideoCapture cap, cv::VideoWriter writer)
{
    using namespace cv;
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
}

void UvcGrabber::free_V4L_Resources(int fd, int n, void** memory, struct v4l2_buffer* buffers)
{
    for (int i = 0; i < n; i++) {
        if (munmap(memory[i], buffers[i].length) < 0) {
            close(fd);
            throw std::runtime_error("Failed to unmap buffer");
        }
    }
    close(fd);
}

void UvcGrabber::free_OpenCV_Resources(cv::VideoWriter writer, cv::VideoCapture cap)
{
    writer.release();
    cap.release();
    cv::destroyAllWindows();
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

    std::string getVideoFolderName(const std::string& folderPath)
    {
        time_t now = time(NULL);
        struct tm* localTime = localtime(&now);
        std::stringstream ss;
        ss << folderPath << "/video_" << localTime->tm_year + 1900 << "-"
           << std::setw(2) << std::setfill('0') << localTime->tm_mon + 1 << "-"
           << std::setw(2) << std::setfill('0') << localTime->tm_mday << "_"
           << std::setw(2) << std::setfill('0') << localTime->tm_hour << "-"
           << std::setw(2) << std::setfill('0') << localTime->tm_min << "-"
           << std::setw(2) << std::setfill('0') << localTime->tm_sec << ".avi";
        return ss.str();
    }
}
