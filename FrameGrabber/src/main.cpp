#include "UvcGrabber.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace
{
	bool directoryExists(const char* path)
	{
		return access(path, F_OK) == 0;
	}

	bool clearDirectory(const char* path)
	{
	    DIR* dir = opendir(path);
		if (dir == NULL) {
		    return false;
		}

		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
		    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
		        continue;
		    }

		    char filePath[256];
		    snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

		    if (remove(filePath) != 0) {
		        closedir(dir);
		        return false;
		    }
		}

		closedir(dir);
		return true;
	}

	bool createDirectory(const char* path)
	{

		return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
	}
}

int main (int argc, char** argv)
{
	if (argc < 3 || argc > 4)
	{
		std::cerr << "Неверное количество параметров!\n";
		exit(EXIT_FAILURE);
	}

	struct Config conf;
	if (argc == 3)
	{
		conf.CameraDeviceName = "/dev/video0";
		conf.FrameRate = atoi(argv[1]);
		conf.FolderName = std::string(argv[2]);
	}
	else 
	{
		conf.CameraDeviceName = std::string(argv[1]);
		conf.FrameRate = atoi(argv[2]);
		conf.FolderName = std::string(argv[3]);
	}

	char path[256];
	std::string fullPath;
	if (getcwd(path, sizeof(path)) != NULL) 
	{
		fullPath = path;
		fullPath += "/" + conf.FolderName;
	    if (directoryExists(fullPath.c_str())) {
	        if (!clearDirectory(fullPath.c_str())) {
	            perror("Failed to clear directory\n");
	            return 1;
	        }
	    } else {
	        if (!createDirectory(fullPath.c_str())) {
	            perror("Failed to create directory\n");
	            return 1;
	        }
	    }
        }
        else {
	    perror("Failed to get current directory\n");
	    return 1;
        }
        conf.FullFolderPath = fullPath;

	

	UvcGrabber grabber(conf);
	if (!grabber.Grab())
		exit(EXIT_FAILURE);
	if (!grabber.AddTimeTag())
		exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
}
