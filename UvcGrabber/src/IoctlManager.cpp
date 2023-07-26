#include "IoctlManager.hpp"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdexcept>

IoctlManager::IoctlManager():
	fd(-1)
{}

IoctlManager::~IoctlManager()
{}

void IoctlManager::setFileDescriptor(int fd)
{
	this->fd = fd;
}

int IoctlManager::getFileDescriptor()
{
	return fd;
}

void IoctlManager::queryCap(void* source)
{
	ioctlFunc(VIDIOC_QUERYCAP, source, std::string("Failed to query capability"));
}

void IoctlManager::setFmt(void* source)
{
	ioctlFunc(VIDIOC_S_FMT, source, std::string("Failed to set format"));
}

void IoctlManager::requestBuffers(void* source)
{
	ioctlFunc(VIDIOC_REQBUFS, source, std::string("Failed to request buffers"));
}	

void IoctlManager::queryBuffer(void* source)
{
	ioctlFunc(VIDIOC_QUERYBUF, source, std::string("Failed to query buffer"));
}

void IoctlManager::queueBuffer(void* source)
{
	ioctlFunc(VIDIOC_QBUF, source, std::string("Failed to queue buffer"));
}

void IoctlManager::startStreaming(void* source)
{
	ioctlFunc(VIDIOC_STREAMON, source, std::string("Failed to start streaming"));
}

void IoctlManager::dequeueBuffer(void* source)
{
	ioctlFunc(VIDIOC_DQBUF, source, std::string("Failed to dequeue buffer"));
}

void IoctlManager::stopStreaming(void* source)
{
	ioctlFunc(VIDIOC_STREAMOFF, source, std::string("Failed to stop streaming"));
}

void IoctlManager::ioctlFunc(long unsigned int request, void* arg, const std::string& errorMsg)
{
	if (ioctl(fd, request, arg) == -1)
    {
        close(fd); 
        throw std::runtime_error(errorMsg.c_str());
    }
}