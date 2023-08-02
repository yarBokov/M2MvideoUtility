#ifndef FOLDER_OPERATIONS_HPP
#define FOLDER_OPERATIONS_HPP

#include <string>
#include <vector>

class FolderOperations
{
	public:
		virtual ~FolderOperations() = default;
		virtual std::string getWorkingDirectory() = 0;
		virtual bool makeFolderForImages(const std::string&) = 0;	
		virtual bool makeFolderForVideo(const std::string&) = 0;
		virtual void deleteDirectoriesExcept(const std::vector < std::string >&) = 0;
		bool makeFolder(const std::string& path);
		bool clearDirectory(const std::string& path);
		bool isDirectoryEmpty(const std::string& path);
};

#endif