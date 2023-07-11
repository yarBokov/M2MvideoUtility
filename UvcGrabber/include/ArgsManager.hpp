#ifndef ARGS_MANAGER_HPP
#define ARGS_MANAGER_HPP

#include <string>
#include <vector>
#include "UvcGrabber.hpp"

class ArgsManager
{
	public:
		ArgsManager() = delete;
		ArgsManager(int argc, char** argv);
		~ArgsManager();
		bool setProgramStrategy(UvcGrabber& grabber);
	private:
		int argc;
		std::vector < std::string > argvVec;
		std::string mode;
		bool validateArgc();
		void invokeClearStrategy(UvcGrabber& grabber);
		void invokeCameraStrategy(UvcGrabber& grabber, bool deviceIsGiven);
};

#endif