#ifndef UVCGRABBER_HPP
#define UVCGRABBER_HPP

#include <string>
#include <vector>
#include <opencv2/core/mat.hpp>

class UvcGrabber
{
	public:
		UvcGrabber();
		~UvcGrabber();

		UvcGrabber(const UvcGrabber& grabber) = delete;
		UvcGrabber& operator=(const UvcGrabber& other) = delete;

		void setDevice(const std::string& deviceName);

		bool GrabFrames(int frames, const std::string& fullFolderPath, int frameWidth = 640 , int frameHeight = 480 );
		bool AddFrameTimeTag(std::string& fullFolderPath, int coordX = 10, int coordY = 30, std::string ext =".jpg");
	private:
		std::string _cameraDeviceName;
		std::vector<std::string> _framesTimeVec;

		void Ioctl(int fd, int request, void* arg, const std::string& errmsg );
		void freeResources(int fd, int n, void** mem, struct v4l2_buffer* buffers);
		cv::Mat getOpenCVImage(const std::string& imagePath);
		void putOpenCVText(int coordX , int coordY, cv::Mat& image, const std::string& text);
		void writeImageOpenCV(const std::string& filePath, cv::Mat& image);
};

#endif
