#ifndef DynamicObject_H
#define DynamicObject_H

#include "IPhysObject.h"
#include "IOgreObject.h"
#include "PhysicsManager.h"
#include <string>

class DynamicObject: public IPhysObject 
{

private:

public:

	int interaction;
	static int numCreated;

	btScalar mass;
	btScalar restitution;

	//Ogre::Entity* ent;
	Ogre::Vector3 scaling;
	Ogre::Vector3 position;
	Ogre::SceneNode* mSceneNode;
	std::list<Ogre::String> meshNames;

	DynamicObject* clone(Ogre::SceneManager* mSceneManager);
	DynamicObject(Ogre::Entity* entity, btRigidBody* rigidBody, btScalar mass, btScalar restitution);
	DynamicObject(std::list<Ogre::String> meshNames, btCollisionShape* collisionShape, Ogre::Vector3 position, int interaction, Ogre::Vector3 scale);
	
	void update();
	void synchWithBullet();
	void addToBullet(PhysicsManager* physmanager);
	void addToOgreScene(Ogre::SceneManager* sceneManager);
	
	void setPosition(Ogre::Vector3 newPos);
	void setOrientation(Ogre::Quaternion newRot);
	void setScale(Ogre::Vector3 v, PhysicsManager* physManager);
};

#endif