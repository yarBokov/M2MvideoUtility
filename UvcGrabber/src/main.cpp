#include "UvcGrabber.hpp"
#include <cstring>
#include "ArgsManager.hpp"
#include "IoctlOperations.hpp"
#include "VideoDevIoctlManager.hpp"
#include <memory>

int main (int argc, char** argv)
{
	std::unique_ptr < IoctlOperations > ioFuncs = std::make_unique < VideoDevIoctlManager >();
	UvcGrabber grabber(std::move(ioFuncs));
	ArgsManager manager(argc, argv);
	if (!manager.setProgramStrategy(grabber))
		return -1;
	return 0;
}
