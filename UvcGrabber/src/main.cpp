#include "UvcGrabber.hpp"
#include <cstring>
#include "ArgsManager.hpp"
#include "IoctlOperations.hpp"
#include "VideoDevIoctlManager.hpp"
#include "FolderOperations.hpp"
#include "GrabberFolderManager.hpp"
#include <memory>
#include <iostream>

int main (int argc, char** argv)
{
	auto ioFuncs = std::make_unique < VideoDevIoctlManager >();
	UvcGrabber grabber(std::move(ioFuncs));
	auto folderOps = std::make_shared < GrabberFolderManager >();
	ArgsManager manager(argc, argv, std::make_shared < GrabberFolderManager >());
	if (!manager.setProgramStrategy(grabber))
		return -1;
	return 0;
}
