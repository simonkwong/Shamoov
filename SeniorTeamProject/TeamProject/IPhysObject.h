#ifndef IPHYSOBJECT_H
#define IPHYSOBJECT_H

#include "OgreEntity.h"
#include "btBulletDynamicsCommon.h"

class IPhysObject 
{

private:

public:

	Ogre::Entity *ent; 
	btCollisionShape *hitBox; 
	btRigidBody *fallRigidBody;

	virtual void synchWithBullet() = 0;
};
#endif