#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <cassert>

#include "System.h"

using namespace irr;

using namespace core;


int main()
{
	System sys;
	sys.initGraphicsContents();
	sys.initPhysicsContents();

	sys.run();

	sys.releasePhysicsContents();
	sys.releaseGraphicsContents();

	return 0;
}