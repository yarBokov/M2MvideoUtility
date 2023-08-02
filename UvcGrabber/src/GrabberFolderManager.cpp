#include "GrabberFolderManager.hpp"
#include <experimental/filesystem>
#include <algorithm>
#include <iostream>

const std::string GrabberFolderManager::WORK_FOLDER_NAME = "/UvcGrabber/";

namespace
{
	namespace fs = std::experimental::filesystem;
}

GrabberFolderManager::GrabberFolderManager():
	_workDirectoryPath(getWorkingDirectory()),
	_imagesDirectoryPath(""),
	_videoDirectoryPath("")
{}

std::string GrabberFolderManager::getWorkingDirectory()
{
	return fs::current_path().string() + WORK_FOLDER_NAME;
}

bool GrabberFolderManager::makeFolderForImages(const std::string& folderName)
{
	std::string imgDir = _workDirectoryPath + folderName;
    if (fs::is_directory(imgDir)) 
    {
        if (clearDirectory(imgDir))
        	_imagesDirectoryPath = imgDir;
    } 
    else if (makeFolder(imgDir))
    	_imagesDirectoryPath = imgDir;
    return true;
}

bool GrabberFolderManager::makeFolderForVideo(const std::string& folderName)
{
	std::string videoDir = _workDirectoryPath + folderName;
    if (fs::is_directory(videoDir)) 
    {
    	_videoDirectoryPath = videoDir;
        return true;
    } 
    else if (makeFolder(videoDir))
    	_videoDirectoryPath = videoDir;
    return true;
}

std::string GrabberFolderManager::getImagesFolderName()
{
	return _imagesDirectoryPath;
}

std::string GrabberFolderManager::getVideoFolderName()
{
	return _videoDirectoryPath;
}

void GrabberFolderManager::deleteDirectoriesExcept(const std::vector < std::string >& directories)
{
	try{
		for (const auto& dir : fs::directory_iterator(_workDirectoryPath))
		{
	        if (fs::is_directory(dir)) 
	        {
	            std::string dirName = dir.path().filename().string();
	            if (std::find(directories.begin(), directories.end(), dirName) == directories.end())
	                fs::remove_all(dir.path());
	            
    		}
		}
	} catch(const fs::filesystem_error& e) {
		std::cerr << "Error occured while deleting directory: " << e.what() << "\n";
	}
}