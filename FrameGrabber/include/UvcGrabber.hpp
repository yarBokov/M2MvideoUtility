#ifndef UVCGRABBER_HPP
#define UVCGRABBER_HPP

#include <string>
#include <vector>

struct Config
{
	std::string CameraDeviceName;
	uint32_t FrameRate;
	std::string FolderName;
	std::string FullFolderPath;
};

class UvcGrabber
{
	public:
		explicit UvcGrabber(const Config& config);
		~UvcGrabber();

		UvcGrabber() = delete;
		UvcGrabber(const UvcGrabber& grabber) = delete;
		UvcGrabber& operator=(const UvcGrabber& other) = delete;

		bool Grab();
		bool AddTimeTag();
	private:
		Config _config;
		std::vector<std::string> _framesTimeVec;

};

#endif
