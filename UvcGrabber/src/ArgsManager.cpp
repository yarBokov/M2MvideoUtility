#include "ArgsManager.hpp"
#include <iostream>
#include <exception>
#include "folder-functions.hpp"

ArgsManager::ArgsManager(int argc, char** argv)
{
	this->argc = argc;
    for (int i = 0; i < argc; ++i) {
        argvVec.push_back(argv[i]);
    }
	mode = argvVec[1];
}

ArgsManager::~ArgsManager()
{}

bool ArgsManager::validateArgc()
{
	if (argc !=  2 && argc != 4 && argc != 5)
	{
		std::cerr << "Неверное количество параметров!\n";
		return false;
	}
	return true;
}

bool ArgsManager::setProgramStrategy(UvcGrabber& grabber)
{
	if (!validateArgc())
		return false;
	try {
		if (argc == 2)
		{
			invokeClearStrategy(grabber);
		}
		else
		{
			bool deviceIsGiven = false;
			std::string mode = argvVec[1];
			if (argc == 5)
			{
				grabber.setDevice(argvVec[2]);
				deviceIsGiven = true;
			}
			invokeCameraStrategy(grabber, deviceIsGiven);
		}
	} catch(std::runtime_error& err) {
		std::cerr << err.what() << "\n";
		return false;
	}
	return true;
}

void ArgsManager::invokeClearStrategy(UvcGrabber& grabber)
{
	if (mode == "clear")
	{
		std::vector < std::string > safeDirs = { "src", "include", "obj" };
		folder_funcs::deleteDirectoriesExcept(folder_funcs::getProgramWorkingDirectory(), safeDirs);
	}
	else
		throw std::runtime_error("Могу принять только команду clear для удаления директорий хранения видео и кадров");
}

void ArgsManager::invokeCameraStrategy(UvcGrabber& grabber, bool deviceIsGiven)
{
	std::string fullFolderPath;
	if (mode == "f" || mode == "frames" || mode == "frame")
	{
		if (!folder_funcs::makeFolder(argvVec[3 + int(deviceIsGiven)].c_str(), fullFolderPath, true))
			throw std::runtime_error("");
		if (!grabber.GrabFrames(atoi(argvVec[2 + int(deviceIsGiven)].c_str()), fullFolderPath))
			throw std::runtime_error("");
		if (!grabber.AddFrameTimeTag(fullFolderPath))
			throw std::runtime_error("");

	}
	else
		std::cerr << "Указан неопознанный режим работы!\n";	
}
