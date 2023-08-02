#ifndef UVCGRABBER_HPP
#define UVCGRABBER_HPP

#include <string>
#include <vector>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <memory>
#include "IoctlOperations.hpp"

class UvcGrabber
{
	public:
		UvcGrabber(std::unique_ptr < IoctlOperations > ioFuncs);
		~UvcGrabber();
		UvcGrabber(const UvcGrabber& grabber) = delete;
		UvcGrabber& operator=(const UvcGrabber& other) = delete;
		void setDevice(const std::string& deviceName);
		bool GrabFrames(int frames, const std::string& fullFolderPath, int frameWidth = 640 , int frameHeight = 480);
		bool AddFrameTimeTag(std::string& fullFolderPath, int coordX = 10, int coordY = 30, std::string ext =".jpg");
		bool CaptureVideo(int duration, const std::string& fullFolderPath);
	private:
		std::string _cameraDeviceName;
		std::vector<std::string> _framesTimeVec;
		std::unique_ptr < IoctlOperations > _ioManager;

		void setFrameFormat(int width, int height, struct v4l2_format* fmt);
		void setRequestedBuffers(unsigned int req_count, struct v4l2_requestbuffers* req);
		void setFrameBuffer(int index, struct v4l2_buffer* buffer);
		void saveAllFrames(int frames, const char* folderPath, void** memory);
		void saveOneFrame(char* filename, void** memory, struct v4l2_buffer* buffer);

		cv::Mat getOpenCVImage(const std::string& imagePath);
		void putOpenCVText(int coordX , int coordY, cv::Mat& image, const std::string& text);
		void writeImageOpenCV(const std::string& filePath, cv::Mat& image);

		void captureFrameByFrame(int duration, cv::VideoCapture cap, cv::VideoWriter writer);

		void free_V4L_Resources(int fd, int n, void** memory, struct v4l2_buffer* buffers);
		void free_OpenCV_Resources(cv::VideoWriter writer, cv::VideoCapture cap);
};
#endif
