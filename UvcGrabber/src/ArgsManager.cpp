#include "ArgsManager.hpp"
#include <iostream>
#include <stdexcept>

ArgsManager::ArgsManager(int argc, char** argv, UvcGrabber grabber, std::shared_ptr < FolderOperations > folderOps)
{
	this->argc = argc;
	folderManager = std::move(folderOps);
    for (int i = 0; i < argc; ++i) {
        argvVec.push_back(argv[i]);
    }
	mode = argvVec[1];
	this->grabber = grabber;
	deviceIsGiven = false;
	frameModeKeys = {std::string("f"), std::string("frame"), std::string("frames")};
	videoModeKeys = {std::string("v"), std::string("video"), std::string("vid")};
}

ArgsManager::~ArgsManager()
{}

bool ArgsManager::validateArgc()
{
	if (argc !=  2 && argc != 4 && argc != 5)
	{
		std::cerr << "Program only supports two, four or five input parameters!\n";
		return false;
	}
	return true;
}

bool ArgsManager::setProgramStrategy()
{
	if (!validateArgc())
		return false;
	try {
		if (argc == 2)
		{
			invokeClearStrategy();
		}
		else
		{
			std::string mode = argvVec[1];
			if (argc == 5)
			{
				grabber.setDevice(argvVec[2]);
				deviceIsGiven = true;
			}
			invokeCameraStrategy();
		}
	} catch(std::runtime_error& err) {
		std::cerr << err.what() << "\n";
		return false;
	}
	return true;
}

void ArgsManager::invokeClearStrategy()
{
	if (mode == "clear")
	{
		std::vector < std::string > safeDirs = { "src", "include", "obj" };
		folderManager->deleteDirectoriesExcept(safeDirs);
	}
	else
		throw std::runtime_error("With this count of parameters program can only accept 'clear' comand");
}

void ArgsManager::invokeCameraStrategy()
{
	std::shared_ptr < GrabberFolderManager > gfmPtr = std::dynamic_pointer_cast < GrabberFolderManager >(folderManager);
	if (frameModeKeys.find(mode) != frameModeKeys.end())
		processFrameMode(gfmPtr);
	else if (videoModeKeys.find(mode) != videoModeKeys.end())
		processVideoMode(gfmPtr);
	else
		throw std::runtime_error("Program only work in frame or video modes. Try again!");	
}

void ArgsManager::processFrameMode(const std::shared_ptr < GrabberFolderManager >& gfmPtr)
{
	if (!folderManager->makeFolderForImages(argvVec[3 + int(deviceIsGiven)]))
		throw std::runtime_error("Error occured while making folder for frames");
	std::string imgDir = gfmPtr->getImagesFolderName();
	if (!grabber.GrabFrames(atoi(argvVec[2 + int(deviceIsGiven)].c_str()), imgDir))
		throw std::runtime_error("Error occured while grabbing frames");
	if (!grabber.AddFrameTimeTag(imgDir))
		throw std::runtime_error("Error occured while adding time tag on frames");
}

void ArgsManager::processVideoMode(const std::shared_ptr < GrabberFolderManager >& gfmPtr)
{
	if (!folderManager->makeFolderForVideo(argvVec[3 + int(deviceIsGiven)]))
		throw std::runtime_error("Error occured while making folder for video");
	std::string videoDir = gfmPtr->getVideoFolderName();
	if (!grabber.CaptureVideo(atoi(argvVec[2 + int(deviceIsGiven)].c_str()), videoDir))
		throw std::runtime_error("Error occured while capturing video");
}
