#ifndef SPHERE_H_
#define SPHERE_H_

class Sphere
{
public:
	btRigidBody* rigidBody;
	irr::scene::ISceneNode* sceneNode;

	Sphere(btRigidBody* rigidBody, irr::scene::ISceneNode* sceneNode);

	void update(double dt);
};

#endif // SPHERE_H_