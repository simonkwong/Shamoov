#ifndef GameLibrary_H
#define GameLibrary_H

#include <stdio.h>
#include <unordered_map>
#include "string"

#include "Stage.h"
#include "DynamicObject.h"
#include "btBulletDynamicsCommon.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using namespace std;

class GameLibrary
{

private:

	unordered_map<string, Stage*> stages;
	unordered_map<string, StaticScenery*> staticScenery;
	unordered_map<string, DynamicObject*> dynamicObjects;

	btTriangleMesh* ogreToBulletMesh(Ogre::MeshPtr mesh);

public:

	GameLibrary(Ogre::SceneManager* sceneManager);
	Ogre::SceneManager* mSceneManager;	
	Ogre::Vector3 parseVector3(const rapidjson::Value& x);

	Stage* getStage(string name);
	DynamicObject* getDynamicObject(string name);
	btVector3 ogreToBulletVector3(Ogre::Vector3 x);
	StaticScenery* getStaticScenery(string name, Ogre::Vector3 position, Ogre::Quaternion orientation);
};

#endif