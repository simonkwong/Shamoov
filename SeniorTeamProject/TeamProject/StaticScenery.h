#ifndef STATICSCENERY_H
#define STATICSCENERY_H
#include "OgreVector3.h"
#include "PhysicsManager.h"
#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreSubMesh.h"

class StaticScenery {
public:
	StaticScenery(Ogre::Entity *mEntity, Ogre::Vector3 position, Ogre::Quaternion orientation, int interaction);
	void addToOgreScene(Ogre::SceneManager *sceneManager);
	void addToBullet(PhysicsManager *physmanager);
	btTriangleMesh* ogreToBulletMesh(Ogre::MeshPtr mesh);
	StaticScenery * clone(Ogre::SceneManager* mSceneManager, Ogre::Vector3 position, Ogre::Quaternion orientation, int interaction);
	Ogre::SceneNode *mSceneNode;
	Ogre::Entity *mEntity;
	
	btCollisionObject* btObj;


	btRigidBody* mRigidBody;

	void synchWithBullet();

};
#endif