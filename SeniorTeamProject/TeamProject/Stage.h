#ifndef STAGE_H
#define STAGE_H
#include <list>
#include "DynamicObject.h"
#include "StaticScenery.h"

class Stage {
public:
	std::list<DynamicObject*> dynObjects;
	std::list<StaticScenery*> staticScenery;
	std::list<Ogre::Light*> lights; 
};

#endif