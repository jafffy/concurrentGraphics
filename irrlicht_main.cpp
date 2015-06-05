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
	if (argc > 1) {
		sscanf_s(argv[1], "%lf", &timeOut);
	}
	System sys(timeOut);

	sys.init();

	sys.run();

	sys.release();

	printf("Physics Exit\n");

	return 0;
}