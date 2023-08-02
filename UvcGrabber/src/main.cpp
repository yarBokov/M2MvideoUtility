#include "UvcGrabber.hpp"
#include <cstring>
#include "ArgsManager.hpp"
#include "IoctlOperations.hpp"
#include "VideoDevIoctlManager.hpp"
#include "FolderOperations.hpp"
#include "GrabberFolderManager.hpp"
#include <memory>

int main (int argc, char** argv)
{
	std::unique_ptr < IoctlOperations > ioFuncs = std::make_unique < VideoDevIoctlManager >();
	UvcGrabber grabber(std::move(ioFuncs));
	std::unique_ptr < FolderOperations > folderOps = std::make_unique < GrabberFolderManager >();
	ArgsManager manager(argc, argv, std::move(folderOps));
	if (!manager.setProgramStrategy(grabber))
		return -1;
	return 0;
}
