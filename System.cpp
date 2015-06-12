#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include <iostream>
#include <cassert>

#include <Windows.h>

#include "System.h"
#include <hash_map>

using namespace irr;
using namespace core;

System::System(const double& timeOut)
	: k_timeOut(timeOut), timer(nullptr), isRunning(true), graphicsHandle(NULL), physicsHandle(NULL)
{
	std::cout << "boost::lockfree::queue is ";
	if (!messageQueue.is_lock_free())
		std::cout << "not ";
	std::cout << "lockfree" << std::endl;

	fopen_s(&flog, "log.txt", "wt");

	if (flog == nullptr) {
		printf("file open failed\n");
	}

	fopen_s(&fmessageQueue, "messageQueue.txt", "wt");

	if (fmessageQueue == nullptr) {
		printf("file open failed\n");
	}
}
System::~System()
{
	if (flog) {
		fclose(flog);
	}

	if (fmessageQueue) {
		fclose(fmessageQueue);
	}
}

bool System::init()
{
	InitializeCriticalSection(&cs);

	return true;
}

void System::release()
{
	update_stat.commit();
	draw_stat.commit();

	printf("update : %lf %lf\ndraw : %lf %lf\n",
		update_stat.meanFPS, update_stat.stddevFPS,
		draw_stat.meanFPS, draw_stat.stddevFPS);

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

	LARGE_INTEGER lastTime;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);

	sys->drawFPSTimer = 0.0;
	sys->drawFPS = 0;

	while (sys->device->run()){
		LARGE_INTEGER currentTime;

		QueryPerformanceCounter(&currentTime);
		double dt = double(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

		lastTime = currentTime;

		sys->draw(dt);
	}

	sys->releaseGraphicsContents();

	sys->isRunning = false;

	WaitForSingleObject(sys->physicsHandle, NULL);

	while (sys->messageQueue.empty() == false) {
		sys->messageQueue.pop();
	}

	printf("Graphics Exit\n");

	return EXIT_SUCCESS;
}

void System::run()
{
	physicsHandle = GetCurrentThread();

	CreateThread(NULL, 0, runGraphics, this, 0, nullptr);

	LARGE_INTEGER lastTime;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);

	globalTimer = 0.0;
	FPSTimer = 0.0;
	FPS = 0;

	initPhysicsContents();

	while (isRunning) {
		LARGE_INTEGER currentTime;

		QueryPerformanceCounter(&currentTime);
		double dt = double(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

		lastTime = currentTime;
		update(dt);
	}

	releasePhysicsContents();
}

void System::runSingleThread()
{
	LARGE_INTEGER lastTime;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);

	globalTimer = 0.0;
	FPSTimer = 0.0;
	FPS = 0;

	drawFPSTimer = 0.0;
	drawFPS = 0;

	initPhysicsContents();
	initGraphicsContents();
	timer = device->getTimer();

	while (device->run()) {
		LARGE_INTEGER currentTime;

		QueryPerformanceCounter(&currentTime);
		double dt = double(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

		lastTime = currentTime;
		update(dt);
		draw(dt);
	}

	releaseGraphicsContents();
	releasePhysicsContents();

	printf("Exit\n");
}

void System::update(double dt)
{
	dynamicsWorld->stepSimulation((float)dt, 10);

	for (unsigned i = 0; i < rigidBodies.size(); ++i) {
		auto body = rigidBodies[i];

		if (body == nullptr) {
			continue;
		}

		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);

		const btVector3& origin = trans.getOrigin();
		sendMessage(new Message(EMT_UPDATE, new UpdatePacket(origin.getX(), origin.getY(), origin.getZ(), i)));
		fprintf(flog, "EMT_UPDATE, %d\n", i);
	}

	if (globalTimer > 0.1) {
		addRigidBody((rand() % SHRT_MAX) / (double)SHRT_MAX, 50, (rand() % SHRT_MAX) / (double)SHRT_MAX, 1.0);

		globalTimer = fmod(globalTimer, 0.1);
	}

	if (FPSTimer > 1.0) {
		printf("Update : %d\n", FPS);

		update_stat.update(FPS);

		FPS = 0;
		FPSTimer = fmod(FPSTimer, 0.1);
	}

	globalTimer += dt;
	FPSTimer += dt;
	++FPS;

	for (unsigned int i = 0; i < rigidBodies.size(); ++i){
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
			sendMessage(new Message(EMT_ERASE, new ErasePacket(i)));
			fprintf(flog, "EMT_ERASE, %d\n", i);
		}
	}

	Sleep(20);
}

void System::draw(double dt)
{	
	auto &localQueue = messageQueue;

	while (localQueue.empty() == false) {
		Message* msg = localQueue.front();
		localQueue.pop();

		assert(msg != nullptr);

		if (msg->type == EMT_INSERT) {
			auto packet = reinterpret_cast<InsertPacket*>(msg->user_data);
			assert(packet);
			addSceneNode(packet->x, packet->y, packet->z, packet->radius, packet->idx);
			fprintf(fmessageQueue, "EMT_INSERT, %d\n", packet->idx);
		} else if (msg->type == EMT_ERASE) {
			auto packet = reinterpret_cast<ErasePacket*>(msg->user_data);			
			eraseSceneNode(packet->idx);
			fprintf(fmessageQueue, "EMT_ERASE, %d\n", packet->idx);
		} else if (msg->type == EMT_UPDATE) {
			auto packet = reinterpret_cast<UpdatePacket*>(msg->user_data);
			updateSceneNode(packet->x, packet->y, packet->z, packet->idx);
			fprintf(fmessageQueue, "EMT_UPDATE, %d\n", packet->idx);
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

	if (drawFPSTimer > 1.0) {
		printf("draw : %d\n", drawFPS);

		draw_stat.update(drawFPS);

		drawFPS = 0;
		drawFPSTimer = fmod(drawFPSTimer, 0.1);
	}

	drawFPSTimer += dt;
	++drawFPS;
}


void System::addRigidBody(double x, double y, double z, double radius)
{
	btCollisionShape* shape = new btSphereShape((float)radius);
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
		assert(unused.empty() == false);
		idx = unused.front();
		unused.pop();
		assert(rigidBodies[idx] == nullptr);
		rigidBodies[idx] = rigidBody;
		timers[idx] = 0.0;
	}

	assert(idx != -1);

	sendMessage(new Message(EMT_INSERT, new InsertPacket(x, y, z, radius, idx)));
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
	auto it = sceneNodes.find(idx);
	if (it == sceneNodes.end()) {
		printf("not found %d\n", idx);
		return;
	}

	it->second->setPosition(vector3df(x, y, z));
}

void System::sendMessage(Message* message)
{
	messageQueue.push(message);

	// flush
	unsigned write_available = messageQueue.write_available();
	while (write_available < 10
		&& messageQueue.empty() == false);
}