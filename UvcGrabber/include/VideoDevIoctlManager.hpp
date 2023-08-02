#ifndef VIDEO_DEV_IOCTL_MANAGER_HPP
#define VIDEO_DEV_IOCTL_MANAGER_HPP

#include "IoctlOperations.hpp"

class VideoDevIoctlManager : public IoctlOperations
{
	public:
		VideoDevIoctlManager();

		void setFileDescriptor(int fd) override;
		int getFileDescriptor() override;

		void queryCap(void* source) override final;
		void setFmt(void* source) override final;
		void requestBuffers(void* source) override final;
		void queryBuffer(void* source) override final;
		void queueBuffer(void* source) override final;
		void startStreaming(void* source) override final;
		void dequeueBuffer(void* source) override final;
		void stopStreaming(void* source) override final;
	private:
		int fd;
};

#endif