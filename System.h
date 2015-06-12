#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <map>
#include <queue>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>

#include "Message.h"
#include "Statistic.h"

struct InsertPacket
{
	double x, y, z, radius;
	unsigned idx;

	InsertPacket(const double &x, const double &y, const double &z, const double &radius, const unsigned &idx)
	: x(x), y(y), z(z), radius(radius), idx(idx)
	{
	}
};

struct ErasePacket
{
	unsigned idx;

	ErasePacket(const unsigned& idx)
	: idx(idx)
	{
	}
};

struct UpdatePacket
{
	double x, y, z;
	unsigned idx;

	UpdatePacket(const double &x, const double &y, const double &z, const unsigned &idx)
	: x(x), y(y), z(z), idx(idx)
	{
	}
};

class System
{
	friend DWORD WINAPI runGraphics(LPVOID parameter);
	const double k_timeOut;

public:
	System(const double &timeOut);
	~System();

	bool init();
	void release();

	bool initGraphicsContents();
	void releaseGraphicsContents();

	bool initPhysicsContents();
	void releasePhysicsContents();

	void run();
	void runSingleThread();

	void update(double dt);
	void draw(double dt);

	void addRigidBody(double x, double y, double z, double radius);
	void addSceneNode(double x, double y, double z, double radius, unsigned idx);
	void eraseSceneNode(unsigned idx);
	void updateSceneNode(double x, double y, double z, unsigned idx);

private:
	void sendMessage(Message* message);

	// Graphics
	irr::IrrlichtDevice* device;
	irr::scene::ISceneManager* smgr;
	irr::video::IVideoDriver* driver;
	
	std::map<unsigned, irr::scene::ISceneNode*> sceneNodes;

	irr::ITimer* timer;

	// Physics
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	btCollisionShape* groundShape;
	btRigidBody* groundRigidBody;

	std::vector<btRigidBody*> rigidBodies;
	std::vector<double> timers;

	std::queue<unsigned> unused;	

	irr::scene::ISceneNode* groundSceneNode;
	irr::scene::ICameraSceneNode* camera;

	double globalTimer;
	double FPSTimer;
	unsigned FPS;
	double drawFPSTimer;
	unsigned drawFPS;

	FPSStatistic update_stat, draw_stat;
	
	bool isRunning;
	CRITICAL_SECTION cs;
	boost::lockfree::spsc_queue<Message*, boost::lockfree::capacity<1024>> messageQueue;

	HANDLE graphicsHandle, physicsHandle;

	// Debug
	FILE* flog;
	FILE* fmessageQueue;
};

#endif // SYSTEM_H_
