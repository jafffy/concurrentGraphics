#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "atomic_vector.h"
#include "Sphere.h"

class System
{
	friend void run(System* sys);
public:
	System();
	~System();

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

	// Physics
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	btCollisionShape* groundShape;
	btCollisionShape* fallShape; // sphere

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
};

#endif // SYSTEM_H_