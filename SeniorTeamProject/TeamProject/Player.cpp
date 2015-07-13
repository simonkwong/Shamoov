#include "Player.h"
#include "DynamicObject.h"
#include "World.h"
#include "OgreVector3.h"
//#include "Receivers.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "InputHandler.h"
//#include <list>
#include <ois/ois.h>
#include <math.h>

//#include "OgreOverlayManager.h"
//#include "OgreOverlay.h"
//#include "OgreFontManager.h"
//#include "OgreTextAreaOverlayElement.h"

#include "PhysicsManager.h"
#include "Kinect.h"
#include "Camera.h"

Player::Player(DynamicObject *dynamic, Ogre::Vector3 position, PhysicsManager *physManager, 
			   World *world, Kinect *k, Ogre::SceneManager *sceneManager, InputHandler *input, GameCamera *camera) : 
mWorld(world), mKinect(k), mSceneManager(sceneManager), mInputHandler(input), mCamera(camera)
{
	mPhysManager = physManager;
	start = false;

	// For Kinect later on 
	initSkel = false;
	initHitBox = false;

	// Create player node using Jordan's Dynamic Object class 
	mPlayerObject = dynamic;
	mPlayerObject->setPosition(position);
	mPlayerObject->addToOgreScene(mSceneManager);

	canJump = true; 
	isJumping = false; 

	// Children nodes for camera in 3rd person perspective 
	mPlayerObject->mSceneNode->setVisible(false, false); // hides jordan mesh but doesn't hide children 
	camNode = mPlayerObject->mSceneNode->createChildSceneNode("player_cam");
	camNode->attachObject(mCamera->mRenderCamera);

	// Sets up Bullet 	
	mPlayerObject->fallRigidBody->setActivationState(DISABLE_DEACTIVATION); 
	mPlayerObject->fallRigidBody->setAngularFactor(btVector3(0.0f,0.01f,0.0f));
	mPlayerObject->fallRigidBody->setDamping(0.8f, .95f);
	mPlayerObject->fallRigidBody->setFriction(0.5f);

	mPlayerObject->addToBullet(mPhysManager); 
	mPlayerObject->synchWithBullet();
}


// Fix for Simon in regards to animation mesh's orientation 
void Player::setAnimation(DynamicObject *p)
{

	// makes a child node of parent node 
	Ogre::SceneNode *childNode = mPlayerObject->mSceneNode->createChildSceneNode("child");
	childNode->flipVisibility(true); // makes this entire thing visible since parent node is invisible

	for (Ogre::String name : p->meshNames) {
		Ogre::Entity *newEnt = mSceneManager->createEntity(name);
		//mEntity->setCastShadows(true);
		childNode->attachObject(newEnt);
		childNode->setPosition(0, -125, 0);
		childNode->setOrientation(Ogre::Quaternion(0, 0, 1, -1)); // does the rotation 
		childNode->roll(Ogre::Radian(Ogre::Math::PI)); // fixes it so player's back faces us 
	}
}

// Sets the scale of player to resize 
void Player::setScale(Ogre::Vector3 v)
{
	mPlayerObject->mSceneNode->setScale(v);
}

// Sets the position 
void Player::setPosition (Ogre::Vector3 p) 
{
	mPlayerObject->setPosition(p);
}

// Basically the update method for the Player class. 
void 
Player::Think(float time)
{

#pragma region Kinect

	//drawSkeleton();

	//drawHitBox("HitBox", mPlayerObject->fallRigidBody);

	//initHitBox = true;

#pragma endregion End of Kinect code/Not used right now   

#pragma region Controls 


	if (start) {

		

		if (canJump)
			playAnimation("idle1", time);

		if (mKinect->getEnableKinect())
		{
			// JUMP
			Ogre::Real jump = mKinect->detectJump();
			if (jump == 0 || jump == 1 || jump == 2)
			{
				playAnimation("fall_idle", time);

				btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
					mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

				if (canJump)
				{
					vector<Ogre::Vector3> skeletonNodes = mKinect->getSkeletonNodes();

					Ogre::Real jumpHeightPercentage = 1.0;

					// LEFT FOOT LIFT JUMP
					if (jump == 0)
					{
						jumpHeightPercentage = Ogre::Math::Abs(mKinect->initialPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y - 
							skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y) / mKinect->leftLegLiftMax;

					}
					// RIGHT FOOT LIFT JUMP
					else if (jump == 1)
					{
						jumpHeightPercentage = Ogre::Math::Abs(mKinect->initialPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y - 
							skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y) / mKinect->rightLegLiftMax;
					}
					// BOTH FEET OFF GROUND JUMP

					else if (jump == 2)
					{
						jumpHeightPercentage = ((Ogre::Math::Abs(mKinect->initialPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y - 
							skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y) + 
							Ogre::Math::Abs(mKinect->initialPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y - 
							skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y)) / 2) / 
							((mKinect->leftLegLiftMax + mKinect->rightLegLiftMax) / 2);
					}

					if (jumpHeightPercentage <= 0.0 || jumpHeightPercentage > 1.2 || jump == -1)
						jumpHeightPercentage = 1.0;

					Ogre::Real jumpHeight = 125 * jumpHeightPercentage;

					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * 40, jumpHeight, currCameraPos.getZ() * 40));			
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * 40, jumpHeight, currCameraPos.getZ() * 40));	
				}
			}

			else if (canJump)
			{
				if (mKinect->detectArm() == 0)
				{
					mWorld->display->displayHint(false);
					mWorld->display->displayEnding(false);
				}

				Ogre::Real sway = mKinect->detectSway();

				// RIGHT
				if (sway > 0.0)
				{
					playAnimation("strafeRight", time);

					Ogre::Real swayRightPercentage = sway / mKinect->swayRightMax;

					if (swayRightPercentage <= 0.0 || swayRightPercentage > 1.2)
						swayRightPercentage = 1.0;

					Ogre::Real strafeRightSpeed = 50 * swayRightPercentage; 

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealRight().x, 
						mCamera->mRenderCamera->getRealRight().y, mCamera->mRenderCamera->getRealRight().z); 

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,0,0));


					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * strafeRightSpeed, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * strafeRightSpeed));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * strafeRightSpeed,
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * strafeRightSpeed));
				}
				// LEFT
				else if (sway < 0.0)
				{
					playAnimation("strafeLeft", time);

					Ogre::Real swayLeftPercentage = sway / mKinect->swayLeftMax;

					if (swayLeftPercentage <= 0.0 || swayLeftPercentage > 1.2)
						swayLeftPercentage = 1.0;

					Ogre::Real strafeLeftSpeed = 50 * swayLeftPercentage; 

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealRight().x, 
						mCamera->mRenderCamera->getRealRight().y, mCamera->mRenderCamera->getRealRight().z); 

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,0,0));
					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * -strafeLeftSpeed, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -strafeLeftSpeed));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * -strafeLeftSpeed,
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -strafeLeftSpeed));

				}

				Ogre::Real turn = mKinect->detectTurn();

				if (turn < 0.0)
				{
					playAnimation("turnLeft", time);

					Ogre::Real turnLeftPercentage = turn / mKinect->leftRotationMax;

					if (turnLeftPercentage <= 0.0 || turnLeftPercentage > 1.2)
						turnLeftPercentage = 1.0;

					Ogre::Real turnLeftSpeed = turnLeftPercentage;

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,-turnLeftSpeed,0));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(0,
						mPhysManager->_world->getGravity().getY() + 70,0));
				}

				else if (turn > 0.0)
				{
					playAnimation("turnRight", time);

					Ogre::Real turnRightPercentage = turn / mKinect->rightRotationMax;

					if (turnRightPercentage <= 0.0 || turnRightPercentage > 1.2)
						turnRightPercentage = 1.0;

					Ogre::Real turnRightSpeed = turnRightPercentage;

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,turnRightSpeed,0));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(0,
						mPhysManager->_world->getGravity().getY() + 70,0));
				}

				Ogre::Real lean = mKinect->detectLean();

				if (lean < 0.0)
				{

					Ogre::Real leanForwardPercentage = Ogre::Math::Abs(lean / mKinect->leanForwardMax);

					if (leanForwardPercentage <= 0.0 || leanForwardPercentage > 1.2)
						leanForwardPercentage = 1.0;

					Ogre::Real forwardSpeed = leanForwardPercentage * 80;

					if (Ogre::Math::Abs(leanForwardPercentage) < 0.6)
					{
						playAnimation("walkForward", time);

						btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
							mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

						mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX(), 
							mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()));

						mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * forwardSpeed, 
							mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * forwardSpeed)); 
					}
					else if (Ogre::Math::Abs(leanForwardPercentage) > 0.6)
					{
						playAnimation("run", time);

						btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
							mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

						mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX(), 
							mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()));

						mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * forwardSpeed, 
							mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * forwardSpeed)); 
					}
				}

				else if (lean > 0.0)
				{
					playAnimation("walkBackward", time);

					Ogre::Real leanBackPercentage = lean / mKinect->leanBackMax;

					if (leanBackPercentage <= 0.0 || leanBackPercentage > 1.2)
						leanBackPercentage = 1.0;

					Ogre::Real backSpeed = leanBackPercentage * 50;

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
						mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * -1, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()) * -1);
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * -backSpeed, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -backSpeed)); 

				}
			}
		}

		// If the keyboard is enabled 
		if (mEnableKeyboard) 
		{	

			// turn off all overlays 
			if (mInputHandler->IsKeyDown(OIS::KC_RETURN))
			{
				mWorld->display->displayHint(false);
				mWorld->display->displayEnding(false);
			}

			// Jump
			if (mInputHandler->IsKeyDown(OIS::KC_SPACE))
			{
				playAnimation("fall_idle", time);

				btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
					mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

				if (canJump)
				{
					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * 90, 125, currCameraPos.getZ() * 90));			
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * 90, 125, currCameraPos.getZ() * 90));			
				}
			}

			if (canJump) 
			{
				// Left 
				if (mInputHandler->IsKeyDown(OIS::KC_M))
				{
					playAnimation("strafeRight", time);

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealRight().x, 
						mCamera->mRenderCamera->getRealRight().y, mCamera->mRenderCamera->getRealRight().z); 

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,0,0));


					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * 40, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * 40));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * 40,
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * 40));
				}

				// Right 
				else if (mInputHandler->IsKeyDown(OIS::KC_N))
				{
					playAnimation("strafeLeft", time);

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealRight().x, 
						mCamera->mRenderCamera->getRealRight().y, mCamera->mRenderCamera->getRealRight().z); 

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,0,0));
					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * -40, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -40));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * -40,
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -40));
				}

				// Does the rotation counter-clockwise

				if (mInputHandler->IsKeyDown(OIS::KC_LEFT))
				{

					playAnimation("turnLeft", time);

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,1,0));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(0,
						mPhysManager->_world->getGravity().getY() + 70,0));
				} 
				// Does rotation clockwise 
				else if (mInputHandler->IsKeyDown(OIS::KC_RIGHT))
				{
					playAnimation("turnRight", time);

					mPlayerObject->fallRigidBody->setAngularVelocity(btVector3(0,-1,0));
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(0,
						mPhysManager->_world->getGravity().getY() + 70,0));
				}

				// Moves Forward 
				if (mInputHandler->IsKeyDown(OIS::KC_UP))
				{
					playAnimation("walkForward", time);

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
						mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX(), 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()));

					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * 40, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * 40)); 
				}

				// Moves Backward 
				else if (mInputHandler->IsKeyDown(OIS::KC_DOWN))
				{

					playAnimation("walkBackward", time);

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
						mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX() * -1, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()) * -1);
					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * -40, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * -40)); 
				}

				else if (mInputHandler->IsKeyDown(OIS::KC_LCONTROL))
				{
					playAnimation("crouch_idle", time);
				}
				else if (mInputHandler->IsKeyDown(OIS::KC_LSHIFT))
				{
					playAnimation("run", time);

					btVector3 currCameraPos = btVector3(mCamera->mRenderCamera->getRealDirection().x, 
						mCamera->mRenderCamera->getRealDirection().y, mCamera->mRenderCamera->getRealDirection().z); 

					mPlayerObject->fallRigidBody->applyCentralImpulse(btVector3(currCameraPos.getX(), 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ()));

					mPlayerObject->fallRigidBody->setLinearVelocity(btVector3(currCameraPos.getX() * 100, 
						mPhysManager->_world->getGravity().getY() + 70, currCameraPos.getZ() * 100)); 
				}
			}

			checkGround(5000.0f, false); // checks if player is within stage 
			checkGround(5.0f, true); // check if player is on the ground or currently jumping  
		}

		mPlayerObject->synchWithBullet();
#pragma endregion Input controls for keyboard 
	}
}

void
	Player::checkGround(float distance, bool checkJump)
{
	btTransform p; 
	mPlayerObject->fallRigidBody->getMotionState()->getWorldTransform(p);
	btVector3 position = p.getOrigin();

	btVector3 btFrom(position.getX(), position.getY(), position.getZ());			// current Player position  
	btVector3 btTo(position.getX(), position.getY() - 20 - distance, position.getZ());	// below Player position 
	btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);

	mPhysManager->_world->rayTest(btFrom, btTo, res); 

	std::string player_y = std::to_string(position.getY());
	//OutputDebugString("Player y position is : \n"); 
	//OutputDebugString(player_y.c_str());
	//OutputDebugString("\n hit length is \n");

	std::string x = std::to_string(res.m_hitPointWorld.getX());
	std::string y = std::to_string(res.m_hitPointWorld.getY());

	//OutputDebugString(y.c_str());
	//OutputDebugString("\n");
	std::string z = std::to_string(res.m_hitPointWorld.getZ());

	if(res.hasHit())
	{
		//OutputDebugString("Has hit so canJump is true?\n");
		canJump = true; 

	} else 
	{
		if (!checkJump)
		{
			// Respawn player 
			p.setOrigin(btVector3(90, 50, -600));
			mPlayerObject->fallRigidBody->setWorldTransform(p);
			//OutputDebugString("respawning player\n");
		} 
		else
		{
			// Player is currently in the air 
			canJump = false; 
			//OutputDebugString("Player is in the air\n");
		}
	}
}

void
	Player::createHitBox(std::string name)
{
	Ogre::ManualObject* myManualObject =  mSceneManager->createManualObject(name); 
	myManualObject->setDynamic(true);
	Ogre::SceneNode* myManualObjectNode = mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_node"); 

	Ogre::MaterialPtr myManualObjectMaterial = Ogre::MaterialManager::getSingleton().create(name + "Material","General"); 
	myManualObjectMaterial->setReceiveShadows(false); 
	myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true);
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(1,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);

	myManualObjectNode->attachObject(myManualObject);
}

void
	Player::drawHitBox(std::string name, btRigidBody *box)
{
	btVector3 min, max;
	box->getAabb(min, max);

	btScalar mX, mY, mZ, MX, MY, MZ;
	mX = min.getX();
	mY = min.getY();
	mZ = min.getZ();

	MX = max.getX();
	MY = max.getY();
	MZ = max.getZ();

	if (!initHitBox)
		createHitBox(name);
	else if (initHitBox)
	{
		Ogre::ManualObject* myManualObject = mSceneManager->getManualObject(name);

		myManualObject->clear();

		myManualObject->begin(name + "Material", Ogre::RenderOperation::OT_LINE_LIST);
		myManualObject->position(mX, mY, mZ);
		myManualObject->position(MX, mY, mZ);

		myManualObject->position(MX, mY, mZ);
		myManualObject->position(MX, mY, MZ);

		myManualObject->position(MX, mY, MZ);
		myManualObject->position(mX, mY, MZ);

		myManualObject->position(mX, mY, MZ);
		myManualObject->position(mX, mY, mZ);

		myManualObject->position(MX, MY, MZ);
		myManualObject->position(mX, MY, MZ);

		myManualObject->position(mX, MY, MZ);
		myManualObject->position(mX, MY, mZ);

		myManualObject->position(mX, MY, mZ);
		myManualObject->position(MX, MY, mZ);

		myManualObject->position(mX, MY, mZ);
		myManualObject->position(MX, MY, MZ);

		myManualObject->position(mX, mY, mZ);
		myManualObject->position(mX, MY, mZ);

		myManualObject->position(MX, mY, mZ);
		myManualObject->position(MX, MY, mZ);

		myManualObject->position(MX, mY, MZ);
		myManualObject->position(MX, MY, MZ);

		myManualObject->position(mX, mY, MZ);
		myManualObject->position(mX, MY, MZ);

		myManualObject->position(mX, mY, mZ);
		myManualObject->position(MX, MY, MZ);

		myManualObject->position(MX, mY, mZ);
		myManualObject->position(mX, MY, MZ);

		myManualObject->position(MX, mY, MZ);
		myManualObject->position(mX, MY, mZ);

		myManualObject->position(mX, mY, MZ);
		myManualObject->position(MX, MY, mZ);

		myManualObject->end();
	}
}

void
	Player::drawSkeleton()
{
	//TORSO
	drawLine("head2ShoulderCenter", NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	drawLine("shoulderCenter2ShoulderLeft", NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	drawLine("shoulderCenter2ShoulderRight", NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	drawLine("shoulderCenter2Spine", NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	drawLine("spine2HipCenter", NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);


	drawLine("hipCenter2HipLeft", NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	drawLine("hipCenter2HipRight", NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	//LEFT ARM
	drawLine("shoulderLeft2ElbowLeft", NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	drawLine("elbowLeft2WristLeft", NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	drawLine("wristLeft2HandLeft", NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	//RIGHT ARM
	drawLine("shoulderRight2ElbowRight", NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	drawLine("elbowRight2WristRight", NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	drawLine("wristRight2HandRight", NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	//LEFT LEG
	drawLine("hipLeft2KneeLeft", NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	drawLine("kneeLeft2AnkleLeft", NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	drawLine("ankleLeft2FootLeft", NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	//RIGHT LEG
	drawLine("hipRight2KneeRight", NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	drawLine("kneeRight2AnkleRight", NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	drawLine("ankleRight2FootRight", NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	initSkel = true;
}

void
	Player::createLine(std::string bone, int joint1, int joint2)
{
	Ogre::ManualObject* myManualObject =  mSceneManager->createManualObject(bone); 
	myManualObject->setDynamic(true);
	Ogre::SceneNode* myManualObjectNode = mSceneManager->getRootSceneNode()->createChildSceneNode(bone + "_node"); 

	Ogre::MaterialPtr myManualObjectMaterial = Ogre::MaterialManager::getSingleton().create(bone + "Material","General"); 
	myManualObjectMaterial->setReceiveShadows(false); 
	myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true);
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(1,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);

	myManualObjectNode->attachObject(myManualObject);

}

void
	Player::drawLine(std::string bone, int joint1, int joint2)
{
	if (!initSkel)
		createLine(bone, joint1, joint2);
	else if (initSkel)
	{
		vector<Ogre::Vector3> skeletonNodes = mKinect->getSkeletonNodes();
		Ogre::ManualObject* myManualObject = mSceneManager->getManualObject(bone);

		myManualObject->clear();

		Ogre::Vector3 bone1 = 20 * skeletonNodes[joint1] + Ogre::Vector3(-50, 30, 20);
		Ogre::Vector3 bone2 = 20 * skeletonNodes[joint2] + Ogre::Vector3(-50, 30, 20);

		myManualObject->begin(bone + "Material", Ogre::RenderOperation::OT_LINE_LIST);
		myManualObject->position(bone1.x, bone1.y, bone1.z); 
		myManualObject->position(bone2.x, bone2.y, bone2.z); 
		myManualObject->end();
	}
}

void
	Player::clearLine(std::string bone)
{
	mSceneManager->destroyManualObject(bone);
	mSceneManager->getRootSceneNode()->removeAndDestroyChild(bone + "_node");
}

/*		crouch_idle | crouch_to_stand | crouch_walk | fall_idle | fall_to_roll
*		gangname_style | idle0 | idle1 | idle2 | jump | turnLeft | turnLeft90
*		turnRight | turnRight90 | run | run_to_stop | samba | strafeLeft
*		strafeRight | the_running_man | walkBackward | walkForward
*		(Example: playAnimation("jump", time);)
*/
void 
	Player::playAnimation(std::string anim, float time)
{
	disableAnimations();

	Ogre::SceneManager::MovableObjectIterator iterator = SceneManager()->getMovableObjectIterator("Entity");
	while(iterator.hasMoreElements())
	{
		Ogre::Entity* e = static_cast<Ogre::Entity*>(iterator.getNext());

		if (e->hasSkeleton())
		{
			Ogre::AnimationState *animation = e->getAnimationState(anim);

			/*if (animation->getAnimationName().compare("jump") == 0)
			finishAnimation(animation, time);
			*/
			animation->setEnabled("true");
			animation->addTime(time);
		}
	}
}

void
	Player::finishAnimation(Ogre::AnimationState *animation, float time)
{
	Ogre::Real animLength = animation->getLength();
	Ogre::Real timePosition = animation->getTimePosition();

	while(!animation->hasEnded())
	{
		animation->addTime(time);
	}
}

void
	Player::stopAnimation(std::string anim)
{
	Ogre::SceneManager::MovableObjectIterator iterator = SceneManager()->getMovableObjectIterator("Entity");
	while(iterator.hasMoreElements())
	{
		Ogre::Entity* e = static_cast<Ogre::Entity*>(iterator.getNext());

		if (e->hasSkeleton())
		{
			Ogre::AnimationState *animation = e->getAnimationState(anim);
			animation->setEnabled(false);
			animation->setTimePosition(0);
		}
	}
}

void
	Player::disableAnimations()
{
	Ogre::SceneManager::MovableObjectIterator iterator = SceneManager()->getMovableObjectIterator("Entity");
	while(iterator.hasMoreElements())
	{
		Ogre::Entity* e = static_cast<Ogre::Entity*>(iterator.getNext());

		if (e->hasSkeleton())
		{
			Ogre::AnimationStateIterator iter = e->getAllAnimationStates()->getAnimationStateIterator();

			while(iter.hasMoreElements())
			{
				Ogre::AnimationState *animState = iter.getNext();

				animState->setEnabled(false);
			}
		}
	}
}
