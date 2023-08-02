#include "VideoDevIoctlManager.hpp"
#include <linux/videodev2.h>
#include <stdexcept>

VideoDevIoctlManager::VideoDevIoctlManager():
	fd(-1)
{}

void VideoDevIoctlManager::setFileDescriptor(int fd)
{
	this->fd = fd;
}

int VideoDevIoctlManager::getFileDescriptor()
{
	return fd;
}

void VideoDevIoctlManager::queryCap(void* source)
{
	ioctlFunction(fd, VIDIOC_QUERYCAP, source, std::string("Failed to query capability"));
}

void VideoDevIoctlManager::setFmt(void* source)
{
	ioctlFunction(fd, VIDIOC_S_FMT, source, std::string("Failed to set format"));
}

void VideoDevIoctlManager::requestBuffers(void* source)
{
	ioctlFunction(fd, VIDIOC_REQBUFS, source, std::string("Failed to request buffers"));
}	

void VideoDevIoctlManager::queryBuffer(void* source)
{
	ioctlFunction(fd, VIDIOC_QUERYBUF, source, std::string("Failed to query buffer"));
}

void VideoDevIoctlManager::queueBuffer(void* source)
{
	ioctlFunction(fd, VIDIOC_QBUF, source, std::string("Failed to queue buffer"));
}

void VideoDevIoctlManager::startStreaming(void* source)
{
	ioctlFunction(fd, VIDIOC_STREAMON, source, std::string("Failed to start streaming"));
}

void VideoDevIoctlManager::dequeueBuffer(void* source)
{
	ioctlFunction(fd, VIDIOC_DQBUF, source, std::string("Failed to dequeue buffer"));
}

void VideoDevIoctlManager::stopStreaming(void* source)
{
	ioctlFunction(fd, VIDIOC_STREAMOFF, source, std::string("Failed to stop streaming"));
}