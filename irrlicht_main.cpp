#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <Windows.h>

#include <cassert>

#include "System.h"

using namespace irr;

using namespace core;


int main(int argc, char* argv[])
{
	double timeOut = 7.0;
	bool isSingleThreaded = false;
	if (argc > 1) {
		sscanf_s(argv[1], "%lf", &timeOut);
	}
	if (argc > 2) {
		char buf[BUFSIZ];
		sscanf_s(argv[2], "%s", buf);

		if (strcmp(buf, "s") == 0) {
			isSingleThreaded = true;
		}
	}
	System sys(timeOut);

	sys.init();

	if (isSingleThreaded) {
		sys.runSingleThread();
	}
	else {
		sys.run();
	}

	sys.release();

	printf("Physics Exit\n");

	return 0;
}