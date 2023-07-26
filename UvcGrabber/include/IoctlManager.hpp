#ifndef IOCTL_MANAGER_HPP
#define IOCTL_MANAGER_HPP

#include <string>


class IoctlManager
{
	public:
		IoctlManager();
		~IoctlManager();
		void setFileDescriptor(int fd);
		int getFileDescriptor();

		void queryCap(void* source);
		void setFmt(void* source);
		void requestBuffers(void* source);
		void queryBuffer(void* source);
		void queueBuffer(void* source);
		void startStreaming(void* source);
		void dequeueBuffer(void* source);
		void stopStreaming(void* source);
	private:
		int fd;
		void ioctlFunc(long unsigned int request, void* arg, const std::string& errorMsg);
};

#endif