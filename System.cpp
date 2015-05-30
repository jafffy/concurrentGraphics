#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <iostream>
#include <cassert>

#include <Windows.h>

#include "System.h"

const double k_timeOut = 7.0;

using namespace irr;
using namespace core;

System::System()
	: timer(nullptr), isRunning(true), graphicsHandle(NULL), physicsHandle(NULL)
{
	std::cout << "boost::lockfree::queue is ";
	if (!messageQueue.is_lock_free())
		std::cout << "not ";
	std::cout << "lockfree" << std::endl;

	fopen_s(&flog, "log.txt", "wt");

	if (flog == nullptr) {
		printf("file open failed\n");
	}
}
System::~System()
{
	if (flog) {
		fclose(flog);
	}
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

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);

	return true;
}
void System::releasePhysicsContents()
{
	for (auto body : rigidBodies) {
		if (body == nullptr) {
			continue;
		}

		dynamicsWorld->removeRigidBody(body);
		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body;
	}

	rigidBodies.clear();

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody->getCollisionShape();
	delete groundRigidBody;
	groundRigidBody = nullptr;

	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
}

DWORD WINAPI runGraphics( LPVOID parameter )
{
	System *sys = (System*)parameter;

	sys->graphicsHandle = GetCurrentThread();
	
	sys->initGraphicsContents();

	sys->timer = sys->device->getTimer();

	while (sys->device->run()){
		sys->draw();
	}

	sys->releaseGraphicsContents();
	EnterCriticalSection(&sys->cs);
	sys->isRunning = false;
	LeaveCriticalSection(&sys->cs);

	WaitForSingleObject(sys->physicsHandle, NULL);

	printf("Graphics Exit\n");

	return EXIT_SUCCESS;
}

void System::run()
{
	physicsHandle = GetCurrentThread();

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

	for (unsigned i = 0; i < rigidBodies.size(); ++i) {
		auto body = rigidBodies[i];

		if (body == nullptr) {
			continue;
		}

		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);

		const btVector3& origin = trans.getOrigin();
		messageQueue.push(new Message(EMT_UPDATE, new UpdatePacket(origin.getX(), origin.getY(), origin.getZ(), i)));
		fprintf(flog, "EMT_UPDATE, %d\n", i);
	}

	if (globalTimer > 0.1) {
		addRigidBody((rand() % SHRT_MAX) / (double)SHRT_MAX, 50, (rand() % SHRT_MAX) / (double)SHRT_MAX, 1.0);
		
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

	for (int i = 0; i < rigidBodies.size(); ++i){
		if (rigidBodies[i] == nullptr) {
			continue;
		}

		timers[i] += dt;
		if (timers[i] > k_timeOut){
			auto body = rigidBodies[i];
			dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body->getCollisionShape();
			delete body;

			rigidBodies[i] = nullptr;
			unused.push(i);
			messageQueue.push(new Message(EMT_ERASE, new ErasePacket(i)));

		}
	}

}
void System::draw()
{
	while (messageQueue.empty() == false) {
		Message* msg = nullptr;
		messageQueue.pop(msg);

		assert(msg != nullptr);

		if (msg->type == EMT_INSERT) {
			auto packet = reinterpret_cast<InsertPacket*>(msg->user_data);
			assert(packet);
			addSceneNode(packet->x, packet->y, packet->z, packet->radius, packet->idx);
		} else if (msg->type == EMT_ERASE) {
			auto packet = reinterpret_cast<ErasePacket*>(msg->user_data);			
			eraseSceneNode(packet->idx);
			
		} else if (msg->type == EMT_UPDATE) {
			auto packet = reinterpret_cast<UpdatePacket*>(msg->user_data);
			updateSceneNode(packet->x, packet->y, packet->z, packet->idx);
		} else {
			printf("Invalid message\n");
			continue;
		}

		delete msg->user_data;
		msg->user_data = nullptr;
		msg->drop();
	}

	driver->beginScene(true, true, video::SColor(255, 255, 0, 255));

	smgr->drawAll();

	driver->endScene();
}


void System::addRigidBody(double x, double y, double z, double radius)
{
	btCollisionShape* shape = new btSphereShape(radius);
	btDefaultMotionState* motionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
	btScalar mass = 1;
	btVector3 Inertia(0, 0, 0);
	shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, Inertia);
	btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
	dynamicsWorld->addRigidBody(rigidBody);

	int idx = -1;

	if (unused.empty()) {
		idx = rigidBodies.size();
		rigidBodies.push_back(rigidBody);
		timers.push_back(0.0);

	} else {
		idx = unused.front();
		unused.pop();
		rigidBodies[idx] = rigidBody;
		timers[idx] = 0.0;
	}

	assert(idx != -1);

	messageQueue.push(new Message(EMT_INSERT, new InsertPacket(x, y, z, radius, idx)));

	fprintf(flog, "EMT_INSERT, %d\n", idx);
}

void System::addSceneNode(double x, double y, double z, double radius, unsigned idx)
{
	scene::ISceneNode* sceneNode = smgr->addSphereSceneNode(radius);
	sceneNode->setPosition(vector3df(x, y, z));
	sceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	sceneNode->setMaterialTexture(0, driver->getTexture("red.png"));

	assert(sceneNode);

	sceneNodes[idx] = sceneNode;
}

void System::eraseSceneNode(unsigned idx)
{
	auto it = sceneNodes.find(idx);
	assert(it != sceneNodes.end());

	it->second->remove();

	sceneNodes.erase(it);
}

void System::updateSceneNode(double x, double y, double z, unsigned idx)
{
	if (sceneNodes.find(idx) == sceneNodes.end()) {
		printf("not found %d\n", idx);
		return;
	}

	sceneNodes[idx]->setPosition(vector3df(x, y, z));
}
