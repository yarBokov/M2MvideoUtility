#include "FolderOperations.hpp"
#include <experimental/filesystem>
#include <iostream>
#include <algorithm>

namespace
{
    namespace fs = std::experimental::filesystem;
}

bool FolderOperations::makeFolder(const std::string& path)
{
    if (!fs::create_directory(path)) 
    {
        std::cerr << "Failed to create directory\n";
        return false;
    }
    return true;
}

bool FolderOperations::clearDirectory(const std::string& path)
{
    try {
        for (const auto& entry : fs::directory_iterator(path)) 
        {
            fs::remove_all(entry.path());
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error occured while deleting file: " << e.what() << "\n";
        return false;
    }
}

bool FolderOperations::isDirectoryEmpty(const std::string& path)
{
    if (!fs::exists(fs::path(path)))
    {
        std::cerr << "Folder " + path + " does not exist!\n";
        return false;
    }
    fs::directory_iterator it(path);
    return fs::begin(it) == fs::end(it);
}