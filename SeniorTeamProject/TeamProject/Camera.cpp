#include "OgreCamera.h"
#include "Camera.h"
#include "World.h"
#include "OgreVector3.h"
#include "Player.h"
// IOS (Input system) header files
#include <ois/ois.h>

// Other input files for my project
#include "InputHandler.h"

GameCamera::GameCamera(Ogre::Camera *renderCamera, InputHandler *input, Ogre::SceneManager *sceneManager) :
mRenderCamera(renderCamera), mInputHandler(input), mSceneManager (sceneManager)
{
	// Sets up mRenderCamera 
	mRenderCamera->setNearClipDistance(1);
    mRenderCamera->setPosition(Ogre::Vector3(0,100,-200));

	//setup();
}

void GameCamera::updatePosition(Ogre::Vector3 camP, Ogre::Vector3 tarP) {
	//mRenderCamera->setPosition(Ogre::Vector3(p.x, p.y + 30, p.z));
	//mRenderCamera->lookAt(p);
   // mCamNode->setPosition (camP);
    //mTargetNode->setPosition (tarP);
}

// Sets up node information 
void GameCamera::setup() 
{
	 mCamNode = mSceneManager->getRootSceneNode ()->createChildSceneNode ("camera");
	 mTargetNode = mSceneManager->getRootSceneNode ()->createChildSceneNode ("camera_target");
	 mCamNode->setAutoTracking (true, mTargetNode); 
	 mCamNode->setFixedYawAxis (false);
	 //mCamNode->attachObject(mRenderCamera);
}

void GameCamera::update(Player *player)
{
	mTargetNode = player->getCameraNode();

	Ogre::Quaternion orientation = mTargetNode->convertLocalToWorldOrientation(mTargetNode->_getDerivedOrientation()); 
	mCamNode->setOrientation(orientation);
	Ogre::Vector3 displacement = mTargetNode->convertLocalToWorldPosition(mTargetNode->_getDerivedPosition());
	mRenderCamera->lookAt(displacement);
	
	//mCamNode->setPosition(Ogre::Vector3(displacement.x, displacement.y, displacement.z));

	// Handles the movement 
	//Ogre::Vector3 displacement;
	//Ogre::Vector3 displacement2;
	//mCamNode->

	//mCamNode->(targetPosition);

	//displacement2 = targetPosition - mTargetNode->getPosition () * 0.5f; 
	//mTargetNode->translate(displacement2);


	//mCamNode->setPosition(displacement);
}

void
GameCamera::Think(float time)
{


	// Any code needed here to move the camera about per frame
	//  (use mRenderCamera to get the actual render camera, of course!)
		
	// Moves the camera up 
	if (mInputHandler->IsKeyDown(OIS::KC_W))
	{
		 Ogre::Vector3 camPos = mRenderCamera->getPosition();
		 mRenderCamera->setPosition(Ogre::Vector3(camPos.x, camPos.y, camPos.z + 1)); 
	}

	// Moves the camera down 
	else if (mInputHandler->IsKeyDown(OIS::KC_S))
	{
		Ogre::Vector3 camPos = mRenderCamera->getPosition();
		mRenderCamera->setPosition(Ogre::Vector3(camPos.x, camPos.y, camPos.z - 1));  
	}

	// Moves the camera left 
	else if (mInputHandler->IsKeyDown(OIS::KC_A))
	{
		Ogre::Vector3 camPos = mRenderCamera->getPosition();
		mRenderCamera->setPosition(Ogre::Vector3(camPos.x + 1, camPos.y, camPos.z));  
	}
	
	// Moves the camera right 
	else if (mInputHandler->IsKeyDown(OIS::KC_D))
	{
		Ogre::Vector3 camPos = mRenderCamera->getPosition();
		mRenderCamera->setPosition(Ogre::Vector3(camPos.x - 1, camPos.y, camPos.z)); 
	}

	// Zooms camera out 
	if (mInputHandler->IsKeyDown(OIS::KC_Q))
	{
		 Ogre::Vector3 camPos = mRenderCamera->getPosition();
		 mRenderCamera->setPosition(Ogre::Vector3(camPos.x, camPos.y + 1, camPos.z)); 

	// Zooms camera in 
	} 
	else if (mInputHandler->IsKeyDown(OIS::KC_E))
	{
		 Ogre::Vector3 camPos = mRenderCamera->getPosition();
		 mRenderCamera->setPosition(Ogre::Vector3(camPos.x, camPos.y - 1, camPos.z)); 
	}  
	
	if (mInputHandler->IsKeyDown(OIS::KC_Z))
	{
		mRenderCamera->yaw(Ogre::Radian(0.01f));

	} else if (mInputHandler->IsKeyDown(OIS::KC_C))
	{
		mRenderCamera->yaw(Ogre::Radian(-0.01f));
	}

	if (mInputHandler->IsKeyDown(OIS::KC_F))
	{
		mRenderCamera->pitch(Ogre::Radian(0.01f));

	} else if (mInputHandler->IsKeyDown(OIS::KC_V))
	{
		mRenderCamera->pitch(Ogre::Radian(-0.01f));
	}

	if (mInputHandler->IsKeyDown(OIS::KC_G))
	{
		mRenderCamera->rotate(Ogre::Quaternion(0, 1, 0, 0));

	} else if (mInputHandler->IsKeyDown(OIS::KC_H))
	{
		mRenderCamera->rotate(Ogre::Quaternion(0, -1, 0, 0));
	}

	if (mInputHandler->IsKeyDown(OIS::KC_G))
	{
		mRenderCamera->rotate(Ogre::Quaternion(0, 0, 1, 0));

	} else if (mInputHandler->IsKeyDown(OIS::KC_H))
	{
		mRenderCamera->rotate(Ogre::Quaternion(0, 0, -1, 0));
	}
}
