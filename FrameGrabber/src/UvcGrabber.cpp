#include "UvcGrabber.hpp"
#include <time.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace
{
    int isDirectoryEmpty(const char* path);

    std::string getTime();
}


UvcGrabber::UvcGrabber(const Config& config):
	_config(config)
{}

UvcGrabber::~UvcGrabber()
{}

bool UvcGrabber::Grab()
{
	int fd = open(_config.CameraDeviceName.c_str(), O_RDWR | O_NONBLOCK);
	if (fd == -1) 
	{// проверить, удалось ли открыть устройство
        perror("Failed to open device");
        return false;
    }

    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    unsigned int n_buffers;
    enum v4l2_buf_type type;

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        perror("Failed to query capability");
        close(fd);
    	return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        printf("Device does not support video capture\n");
        close(fd);
        return false;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 1920;
    fmt.fmt.pix.height = 1080;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        perror("Failed to set format");
        close(fd);
    	return false;
    }


    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        perror("Failed to request buffers");
        close(fd);
        return false;
    }

    struct v4l2_buffer buffers[4];
    void* mem[4];
    n_buffers = req.count;

    for (unsigned int i = 0; i < n_buffers; ++i) 
    {
        buffers[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffers[i].memory = V4L2_MEMORY_MMAP;
        buffers[i].index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buffers[i]) == -1) {
            perror("Failed to query buffer");
            close(fd);
            return false;
        }

        mem[i] = mmap(NULL, buffers[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffers[i].m.offset);
        if (mem[i] == MAP_FAILED) {
            perror("Failed to mmap buffer");
            close(fd);
            return false;
        }

        if (ioctl(fd, VIDIOC_QBUF, &buffers[i]) == -1) {
            perror("Failed to queue buffer");
            close(fd);
            return false;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Failed to start streaming");
        close(fd);
        return false;
    }

    int count = 1;

    while (count <= _config.FrameRate)
    {
    	fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                continue;
            fprintf(stderr, "select error %d, %s\n", errno, strerror(errno));
            return false;
        }
        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            return false;
        } 

        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("Failed to dequeue buffer");
            close(fd);
            return false;
        }

        char filename[256];
        snprintf(filename, sizeof(filename), "%s/frame%d.jpg", _config.FolderName.c_str(), count);

        FILE* file = fopen(filename, "wb");
        if (file == NULL)
        {
            fprintf(stderr, "failed to open file\n");
            close(fd);
            return false;
        } 

        _framesTimeVec.push_back(getTime());

        fwrite(mem[buf.index], buf.bytesused, 1, file);
        fclose(file);

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) 
        {
            perror("Failed to queue buffer");
            close(fd);
            return false;
        }

    	count++;
    }

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("Failed to stop streaming");
        close(fd);
        return false;
    }

    for (int i = 0; i < 2; i++) {
        if (munmap(mem[i], buffers[i].length) < 0) {
            perror("Failed to unmap buffer");
            close(fd);
            return false;
        }
    }

    close(fd);

    return true;
}

bool UvcGrabber::AddTimeTag()
{
    int dir_err = isDirectoryEmpty(_config.FullFolderPath.c_str());
    if (dir_err == -1)
        return false;

    if (dir_err == 0)
    {
        std::cout << "Директория пуста\n";
        return false;
    }

    DIR* dir = opendir(_config.FullFolderPath.c_str());
    if (dir == NULL)
    {
        perror("Error opening directory of images");
        return false;
    }

    struct dirent* entry;
    int frameTimeIndex = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string filename = entry->d_name;
        if (filename.length() < 4 || filename.substr(filename.length() -4) != ".jpg")
        {
            continue;
        }

        std::string filePath = _config.FullFolderPath + "/" + filename;
        cv::Mat image;
        image = cv::imread(filePath);

        std::string date = _framesTimeVec[frameTimeIndex++];

        cv::putText(image, date, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(51, 102, 0), 1);
        cv::imwrite(filePath, image);
    }
    closedir(dir);
    return true;
}
namespace
{
    int isDirectoryEmpty(const char* path)
    {
        DIR* dir = opendir(path);
        if (dir == NULL) {
            perror("Failed to check directory emptiness");
            return -1;
        }

        int status = 0;
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            // Пропускаем текущую и родительскую директории
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Обнаружен элемент, директория не пуста
            status = 1;
            break;
        }

        closedir(dir);
        return status;
    }

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
