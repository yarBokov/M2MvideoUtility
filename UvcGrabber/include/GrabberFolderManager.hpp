#ifndef GRABBER_FOLDER_FUNCTIONS_HPP
#define GRABBER_FOLDER_FUNCTIONS_HPP

#include "FolderOperations.hpp"
#include <string>

class GrabberFolderManager : public FolderOperations
{
	public:
		GrabberFolderManager();
		std::string getWorkingDirectory() override final;
		bool makeFolderForImages(const std::string&) override;		
		bool makeFolderForVideo(const std::string&) override;
		std::string getImagesFolderName();
		std::string getVideoFolderName();
		void deleteDirectoriesExcept(const std::vector < std::string >&) override;
		static const std::string WORK_FOLDER_NAME;
	private:
		std::string _workDirectoryPath;
		std::string _imagesDirectoryPath;
		std::string _videoDirectoryPath;
};

#endif