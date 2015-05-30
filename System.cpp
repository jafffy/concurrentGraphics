#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <cassert>

#include <Windows.h>

#include "System.h"


using namespace irr;
using namespace core;

System::System()
	: timer(nullptr), isRunning(true)
{

}
System::~System()
{

}

bool System::init()
{
	InitializeCriticalSection(&cs);

	return true;
}

void System::release()
{
	DeleteCriticalSection(&cs);
}

bool System::initGraphicsContents()
{
	device = createDevice(video::EDT_OPENGL, dimension2d<u32>(800, 600), 32, false, false, false, nullptr);

	smgr = device->getSceneManager();
	driver = device->getVideoDriver();

	groundSceneNode = smgr->addCubeSceneNode(1.0f);
	groundSceneNode->setPosition(vector3df(0, -1, 0));
	groundSceneNode->setScale(vector3df(100, 0, 100));
	groundSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	groundSceneNode->setMaterialTexture(0, driver->getTexture("blue.png"));

	fallSceneNode = smgr->addSphereSceneNode(1.0f);
	fallSceneNode->setPosition(vector3df(0, 50, 0));
	fallSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	fallSceneNode->setMaterialTexture(0, driver->getTexture("red.png"));

	camera = smgr->addCameraSceneNodeFPS();
	camera->setPosition(vector3df(0, 20, -50));
	camera->setTarget(vector3df(0, 0, 0));

	return true;
}
void System::releaseGraphicsContents()
{
	device->drop();
}

bool System::initPhysicsContents()
{
	broadphase = new btDbvtBroadphase();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	
	solver = new btSequentialImpulseConstraintSolver();

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	fallShape = new btSphereShape(1);

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);

	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);

	rigidBodies.push_back(new Sphere(fallRigidBody, fallSceneNode));
	rigidBodies.push_back(new Sphere(groundRigidBody, groundSceneNode));

	return true;
}
void System::releasePhysicsContents()
{
	for (auto body : rigidBodies) {
		Sphere* sphere = body.get();
		btRigidBody* rigidBody = sphere->rigidBody;

		dynamicsWorld->removeRigidBody(rigidBody);
		delete rigidBody->getMotionState();
		delete rigidBody;

		delete sphere;
	}

	rigidBodies.clear();

	delete fallShape;

	delete groundShape;


	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
}

DWORD WINAPI runGraphics( LPVOID parameter )
{
	System *sys = (System*)parameter;
	
	sys->initGraphicsContents();

	sys->timer = sys->device->getTimer();

	while (sys->device->run()){
		sys->draw();
	}

	sys->releaseGraphicsContents();

	sys->isRunning = false;

	return EXIT_SUCCESS;
}

void System::run()
{
	CreateThread(
		NULL,
		0,
		runGraphics,
		this,
		0, nullptr);

	while (timer == nullptr);

	u32 lastTime = timer->getRealTime();

	globalTimer = 0.0;
	FPSTimer = 0.0;
	FPS = 0;

	initPhysicsContents();
	

	while (isRunning) {
		u32 currentTime = timer->getRealTime();
		double dt = (currentTime - lastTime) / 1000.0;

		lastTime = currentTime;
		update(dt);
	}

	releasePhysicsContents();
}

void System::update(double dt)
{
	dynamicsWorld->stepSimulation(dt, 10);

	if (globalTimer > 0.1) {
		addSphereBody((rand() % SHRT_MAX) / (double)SHRT_MAX, 50, (rand() % SHRT_MAX) / (double)SHRT_MAX, 1.0);

		globalTimer = 0.0;
	}

	if (FPSTimer > 1.0) {
		printf("%d\n", FPS);

		FPS = 0;
		FPSTimer = 0.0;
	}

	globalTimer += dt;
	FPSTimer += dt;
	++FPS;
}
void System::draw()
{
	for (auto body : rigidBodies) {
		btTransform trans;
		body.get()->rigidBody->getMotionState()->getWorldTransform(trans);
		const btVector3& origin = trans.getOrigin();
		body.get()->sceneNode->setPosition(vector3df(origin.getX(), origin.getY(), origin.getZ()));
	}

	driver->beginScene(true, true, video::SColor(255, 255, 0, 255));

	smgr->drawAll();

	driver->endScene();
}

void System::addSphereBody(double x, double y, double z, double radius)
{
	scene::ISceneNode* sceneNode = smgr->addSphereSceneNode(radius);
	sceneNode->setPosition(vector3df(x, y, z));
	sceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	sceneNode->setMaterialTexture(0, driver->getTexture("red.png"));

	btCollisionShape* shape = new btSphereShape(radius);
	btDefaultMotionState* motionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
	btScalar mass = 1;
	btVector3 Inertia(0, 0, 0);
	shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, Inertia);
	btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
	dynamicsWorld->addRigidBody(rigidBody);

	rigidBodies.push_back(new Sphere(rigidBody, sceneNode));
}