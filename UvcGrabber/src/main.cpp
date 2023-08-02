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
	UvcGrabber grabber(std::make_shared < VideoDevIoctlManager >());
	ArgsManager manager(argc, argv, grabber, std::make_shared < GrabberFolderManager >());
	if (!manager.setProgramStrategy())
		return -1;
	return 0;
}
