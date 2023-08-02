#ifndef IOCTL_OPERATIONS_HPP
#define IOCTL_OPERATIONS_HPP

#include <string>

class IoctlOperations
{
	public:
		virtual ~IoctlOperations() = default;
		virtual void setFileDescriptor(int) = 0;
		virtual int getFileDescriptor() = 0;

		virtual void queryCap(void*) = 0;
		virtual void setFmt(void*) = 0;
		virtual void requestBuffers(void*) = 0;
		virtual void queryBuffer(void*) = 0;
		virtual void queueBuffer(void*) = 0;
		virtual void startStreaming(void*) = 0;
		virtual void dequeueBuffer(void*) = 0;
		virtual void stopStreaming(void*) = 0;
		void ioctlFunction(int, long unsigned int, void*, const std::string&);
};

#endif