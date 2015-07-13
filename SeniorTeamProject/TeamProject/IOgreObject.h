#ifndef IOGREOBJECT_H
#define IOGREOBJECT_H

#include <list>
#include <string>
#include "OgreEntity.h"
#include "OgreVector3.h"
#include "OgreSceneManager.h"

class IOgreObject 
{

private:

public:

	Ogre::Vector3 position;
	Ogre::SceneNode *mSceneNode;
	std::list<std::string> meshNames;

	virtual void setPosition(Ogre::Vector3 newPos) = 0;
	virtual void addToOgreScene(Ogre::SceneManager *sceneManager) = 0;
};

#endif