#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <Windows.h>

#include <cassert>

#include "System.h"

using namespace irr;

using namespace core;


int main()
{
	System sys;

	sys.init();

	sys.run();

	sys.release();

	return 0;
}