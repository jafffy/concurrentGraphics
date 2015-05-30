#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <map>
#include <queue>

#include "atomic_vector.h"
#include "Sphere.h"

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

	void addSphereBody(double x, double y, double z, double radius);

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
	btCollisionShape* fallShape; // sphere

	std::vector<btRigidBody*> rigidBodies;
	std::queue<

	btRigidBody* groundRigidBody;
	btRigidBody* fallRigidBody;
	atomic_vector<Sphere*> rigidBodies;

	irr::scene::ISceneNode* groundSceneNode;
	irr::scene::ISceneNode* fallSceneNode;
	irr::scene::ICameraSceneNode* camera;

	double globalTimer;
	double FPSTimer;
	unsigned FPS;

	bool isRunning;
	CRITICAL_SECTION cs;
};

#endif // SYSTEM_H_