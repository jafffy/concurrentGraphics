#include <cstdio>
#include <cstdlib>

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

#include <thread>
#include <mutex>

#include <btBulletDynamicsCommon.h>

#include "Sphere.h"

static bool g_Running = true;
GLFWwindow* window;
std::mutex m{};

class System
{
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	btCollisionShape* groundShape;
	btCollisionShape* fallShape;

	btDefaultMotionState* groundMotionState;
	btRigidBody* groundRigidBody;
	btDefaultMotionState* fallMotionState;
	btRigidBody* fallRigidBody;
public:
	System()
		: sphere(nullptr)
	{
	}

	~System()
	{
	}

	void init()
	{
		broadphase = new btDbvtBroadphase();

		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);

		solver = new btSequentialImpulseConstraintSolver;

		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -10, 0));


		groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

		fallShape = new btSphereShape(1);


		groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
		btRigidBody::btRigidBodyConstructionInfo
			groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
		groundRigidBody = new btRigidBody(groundRigidBodyCI);
		dynamicsWorld->addRigidBody(groundRigidBody);


		fallMotionState =
			new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		fallShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
		fallRigidBody = new btRigidBody(fallRigidBodyCI);
		dynamicsWorld->addRigidBody(fallRigidBody);

		sphere = new Sphere();
	}

	void release()
	{
		delete sphere;
		sphere = nullptr;

		dynamicsWorld->removeRigidBody(fallRigidBody);
		delete fallRigidBody->getMotionState();
		delete fallRigidBody;

		dynamicsWorld->removeRigidBody(groundRigidBody);
		delete groundRigidBody->getMotionState();
		delete groundRigidBody;


		delete fallShape;

		delete groundShape;


		delete dynamicsWorld;
		delete solver;
		delete collisionConfiguration;
		delete dispatcher;
		delete broadphase;
	}

	void update()
	{
		if (sphere) {
			dynamicsWorld->stepSimulation(0.0001f, 10);

			btTransform trans;
			fallRigidBody->getMotionState()->getWorldTransform(trans);
			const btVector3 &pos = trans.getOrigin();
			sphere->setPosition(glm::vec3(pos.getX(), pos.getY(), pos.getZ()));

			printf("%f %f %f\n", pos.getX(), pos.getY(), pos.getZ());
		}
	}

	void draw()
	{
		if (sphere) {
			sphere->draw();
		}
	}

	Sphere* sphere;
};

void work(System* system)
{
	if (!glfwInit()) {
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	system->init();

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		system->draw();

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	system->release();

	m.lock();
	g_Running = false;
	m.unlock();

	glfwTerminate();
}

int main(int argc, char* argv[])
{
	System sys;

	std::thread workerThread { work, &sys };

	while (g_Running) {
		sys.update();
	}

	workerThread.join();

	return 0;
}