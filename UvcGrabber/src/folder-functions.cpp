#include "folder-functions.hpp"
#include <cstring>
#include <iostream>
#include <experimental/filesystem>
#include <algorithm>

namespace folder_funcs
{
	namespace fs = std::experimental::filesystem;

	static const std::string WORK_FOLDER_NAME = "/UvcGrabber/";

	bool clearDirectory(const std::string& path)
	{
		try {
			for (const auto& entry : fs::directory_iterator(path)) 
			{
	        	fs::remove_all(entry.path());
	    	}
    	} catch (const fs::filesystem_error& e) {
    		std::cerr << "Error occured while deleting file: " << e.what() << "\n";
    		return false;
    	}
		return true;
	}
	
    bool isDirectoryEmpty(const std::string& path)
    {
    	if (!fs::exists(fs::path(path)))
    	{
	        std::cerr << "Folder " + path + " does not exist!\n";
        	return false;
    	}
    	fs::directory_iterator it(path);
    	return fs::begin(it) == fs::end(it);
    }

    bool makeFolder(const char* folderName, std::string& fullPath, bool toClear)
    {
    	fullPath = getProgramWorkingDirectory() + std::string(folderName);
	    if (fs::is_directory(fullPath)) 
	    {
	    	if (!toClear)
	    	{
	    		return true;
	    	}
	        if (!folder_funcs::clearDirectory(fullPath))
	            return false;
	    } 
	    else 
	    {
	        if (!fs::create_directory(fullPath)) 
	        {
	            std::cerr << "Failed to create directory\n";
	            return false;
	        }
	    }
	    return true;
    }

    void deleteDirectoriesExcept(const std::string& hostDirectory, std::vector < std::string > directories)
    {
    	try{
    		for (const auto& dir : fs::directory_iterator(hostDirectory))
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

    std::string getProgramWorkingDirectory()
    {
    	return fs::current_path().string() + WORK_FOLDER_NAME;
    }
}