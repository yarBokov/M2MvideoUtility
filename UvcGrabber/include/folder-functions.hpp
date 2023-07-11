#ifndef FOLDER_FUNCTIONS_HPP
#define FOLDER_FUNCTIONS_HPP

#include <string>
#include <vector>

namespace folder_funcs
{
	bool clearDirectory(const std::string&  path);
	bool isDirectoryEmpty(const std::string&  path);
	bool makeFolder(const char* folderName, std::string& fullPath, bool toDelete);
	void deleteDirectoriesExcept(const std::string& hostDirectory, std::vector < std::string > directories);
    std::string getProgramWorkingDirectory();
}


#endif
