#include <irrlicht.h>

#include <btBulletDynamicsCommon.h>

#include "Sphere.h"

Sphere::Sphere(btRigidBody* rigidBody, irr::scene::ISceneNode* sceneNode)
	: rigidBody(rigidBody), sceneNode(sceneNode)
{

}

void Sphere::update(double dt)
{

}