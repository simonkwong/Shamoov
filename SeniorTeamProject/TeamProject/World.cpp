#include "World.h"

// Ogre header files
#include "Ogre.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

//Project headers
#include <list>
#include <stdlib.h>
#include "HUD.h"
#include "Camera.h"
#include "Kinect.h"
#include "Player.h"
#include "InputHandler.h"

using namespace rapidjson;

World::World(Ogre::SceneManager *sceneManager, InputHandler *input, Kinect *sensor, GameCamera *gameCamera, GameLibrary *gameLib, Ogre::Root *mRoot, HUD * hud)   : 
	mSceneManager(sceneManager), mInputHandler(input), mKinect(sensor), mCamera(gameCamera), gameLibrary(gameLib), display(hud)
{

	sceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	sceneManager->setSkyBox(true, "Skybox/Cloudy");

	Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
	lightdir.normalise();

	Ogre::Light* light = sceneManager->createLight("TestLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));



	physManager = new PhysicsManager();

	// Causes shadowcaster error, vertex limit exceeded?
	// Fixed by turning off shadows... but thats lame

	
	DynamicObject *p = gameLibrary->getDynamicObject("Human");
	DynamicObject *j = gameLibrary->getDynamicObject("Jordan");
	

	mPlayer = new Player(j, Ogre::Vector3(-2645, 58, -918), physManager, this, mKinect, mSceneManager, mInputHandler, mCamera);
	mPlayer->setAnimation(p);
	mPlayer->setScale(Ogre::Vector3(.25, .25, .25));

	

	stage = gameLibrary->getStage("UnitTestStage");
	
	for (std::list<DynamicObject*>::iterator it = stage->dynObjects.begin(); it != stage->dynObjects.end(); it++) {
		it._Ptr->_Myval->addToOgreScene(mSceneManager);
	 	it._Ptr->_Myval->addToBullet(physManager);
		it._Ptr->_Myval->setScale(it._Ptr->_Myval->scaling, this->physManager);
	}

	StaticScenery *camLookIt;
	for (std::list<StaticScenery*>::iterator it = stage->staticScenery.begin(); it != stage->staticScenery.end(); it++) {
		it._Ptr->_Myval->addToOgreScene(mSceneManager);
	 	it._Ptr->_Myval->addToBullet(physManager);
		camLookIt = it._Ptr->_Myval;
	}

	// TODO: Fix this so it's not hardcoded
	mCamera->mRenderCamera->lookAt(camLookIt->mSceneNode->getPosition());
	mCamera->mRenderCamera->pitch(Ogre::Radian(0.2f));

	createWater();
	paused = false;

}


void 
World::Think(float time)
{
	doWaterStuff(time);



	/*
	for (std::list<DynamicObject*>::iterator it = stage->dynObjects.begin(); it != stage->dynObjects.end(); it++) {
		mPlayer->drawHitBox(it._Ptr->_Myval->mSceneNode->getName(), it._Ptr->_Myval->fallRigidBody);
	}
	*/

	if (paused)
		return;
	else
		mPlayer->Think(time);


	physManager->stepSimulation(time, this);
}

#pragma region Water

void
World::createWater()
{

#define FLOW_SPEED 0.8
#define FLOW_HEIGHT 4

	Ogre::Entity *mWaterEntity;
	Ogre::Plane mWaterPlane;

	mWaterPlane.normal = Ogre::Vector3::UNIT_Y;
	mWaterPlane.d = -1.5;

	Ogre::MeshManager::getSingleton().createPlane(
		"WaterPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		mWaterPlane, 10000, 10000, 10, 100, true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

	mWaterEntity = mSceneManager->createEntity("water", "WaterPlane");
	mWaterEntity->setMaterialName("Water/11");

	Ogre::SceneNode *mWaterNode = mSceneManager->getRootSceneNode()->createChildSceneNode("WaterNode");
	mWaterNode->attachObject(mWaterEntity);
	mWaterNode->setPosition(0, -5, 0);
	mWaterNode->setOrientation(Ogre::Quaternion::IDENTITY);
	//mWaterNode->setOrientation(0, 0, 1, 0);
	//mWaterNode->translate(3000, -50, 3000);

	
	Ogre::Light *mLight = mSceneManager->createLight("WaterLight");
	mLight->setType(Ogre::Light::LT_DIRECTIONAL);
	mLight->setDirection(-100, 0, 0);
	
}


void
World::doWaterStuff(float time)
{
	float waterFlow = FLOW_SPEED * time;
	static float flowAmount = 0.0f;
	static bool flowUp = true;

	// Ogre::SceneNode *mCamera = mPlayer->getCameraNode();

	Ogre::SceneNode *mWaterNode = static_cast<Ogre::SceneNode*>
		(mSceneManager->getRootSceneNode()->getChild("WaterNode"));

	//mWaterNode->setOrientation(0, 0, 1, 0);

	if (mWaterNode)
	{
		if (flowUp)
			flowAmount += waterFlow;
		else
			flowAmount -= waterFlow;

		if (flowAmount >= FLOW_HEIGHT)
			flowUp = false;
		else if (flowAmount <= 0.0f)
			flowUp = true;

		mWaterNode->translate(0, (flowUp ? waterFlow : -waterFlow), 0);
	}
}

#pragma endregion Water



