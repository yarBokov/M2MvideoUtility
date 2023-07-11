#include "UvcGrabber.hpp"
#include <cstring>
#include "ArgsManager.hpp"

int main (int argc, char** argv)
{
	UvcGrabber grabber;
	ArgsManager manager(argc, argv);
	if (!manager.setProgramStrategy(grabber))
		return -1;
	return 0;
}
