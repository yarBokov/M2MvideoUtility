#include "IoctlOperations.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>

void IoctlOperations::ioctlFunction(int fd, long unsigned int request, void* arg, const std::string& errorMsg)
{
	if (ioctl(fd, request, arg) == -1)
    {
        close(fd); 
        throw std::runtime_error(errorMsg.c_str());
    }
}