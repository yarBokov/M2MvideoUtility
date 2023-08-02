#ifndef ARGS_MANAGER_HPP
#define ARGS_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include "UvcGrabber.hpp"
#include "FolderOperations.hpp"

class ArgsManager
{
	public:
		ArgsManager() = delete;
		ArgsManager(int argc, char** argv, std::unique_ptr < FolderOperations > folderOps);
		~ArgsManager();
		bool setProgramStrategy(UvcGrabber& grabber);
	private:
		int argc;
		std::vector < std::string > argvVec;
		std::unique_ptr < FolderOperations > folderManager;
		std::string mode;
		bool validateArgc();
		void invokeClearStrategy(UvcGrabber& grabber);
		void invokeCameraStrategy(UvcGrabber& grabber, bool deviceIsGiven);
};

#endif