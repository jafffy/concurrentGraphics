#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <map>
#include <queue>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

#include "Message.h"

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
public:
	System();
	~System();

	bool init();
	void release();

	bool initGraphicsContents();
	void releaseGraphicsContents();

	bool initPhysicsContents();
	void releasePhysicsContents();

	void run();

	void update(double dt);
	void draw();

	void addRigidBody(double x, double y, double z, double radius);
	void addSceneNode(double x, double y, double z, double radius, unsigned idx);
	void eraseSceneNode(unsigned idx);
	void updateSceneNode(double x, double y, double z, unsigned idx);

private:
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
	std::queue<unsigned> unused;	

	irr::scene::ISceneNode* groundSceneNode;
	irr::scene::ICameraSceneNode* camera;

	double globalTimer;
	double FPSTimer;
	unsigned FPS;

	bool isRunning;
	CRITICAL_SECTION cs;
	boost::lockfree::queue<Message*, boost::lockfree::capacity<50>> messageQueue;
};

#endif // SYSTEM_H_
