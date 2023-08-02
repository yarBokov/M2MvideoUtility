#ifndef ARGS_MANAGER_HPP
#define ARGS_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include "UvcGrabber.hpp"
#include "FolderOperations.hpp"
#include "GrabberFolderManager.hpp"

class ArgsManager
{
	public:
		ArgsManager() = delete;
		ArgsManager(int argc, char** argv, UvcGrabber grabber, std::shared_ptr < FolderOperations > folderOps);
		~ArgsManager();
		bool setProgramStrategy();
	private:
		int argc;
		std::vector < std::string > argvVec;
		UvcGrabber grabber;
		std::shared_ptr < FolderOperations > folderManager;
		std::string mode;
		bool deviceIsGiven;
		std::unordered_set < std::string > frameModeKeys;
		std::unordered_set < std::string > videoModeKeys;
		bool validateArgc();
		void invokeClearStrategy();
		void invokeCameraStrategy();
		void processFrameMode(const std::shared_ptr < GrabberFolderManager >& gfmPtr);
		void processVideoMode(const std::shared_ptr < GrabberFolderManager >& gfmPtr);
};

#endif