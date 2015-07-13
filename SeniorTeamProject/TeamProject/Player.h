#ifndef __PLAYER_H_
#define __PLAYER_H_


#include "OgreVector3.h"
#include "Receivers.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include <ois/ois.h>
//#include "OgreOverlayManager.h"
//#include "OgreOverlay.h"
//#include "OgreFontManager.h"
//#include "OgreTextAreaOverlayElement.h"
#include "PhysicsManager.h"
#include "DynamicObject.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

// Forward declarations of Ogre classes.  Note the Ogre namespace!
namespace Ogre {
    class SceneNode;
    class SceneManager;
}

class World;
class Kinect;
class DynamicObject;
class InputHandler;
class GameCamera;

class Player 
{
public:

	Player(DynamicObject *dynamic, Ogre::Vector3 position, PhysicsManager *physManager, World *world, 
		Kinect *k, Ogre::SceneManager *sceneManager, InputHandler *input, GameCamera *camera);

	void addToScene(); 
	void setScale(Ogre::Vector3 v);
	void setOrientation(Ogre::Quaternion newOrientation);
	void checkGround(float distance, bool checkJump);

	void Think(float time);
	Ogre::SceneManager *SceneManager() { return mSceneManager; }

	void playAnimation(std::string anim, float time);
	void stopAnimation(std::string anim);
	void finishAnimation(Ogre::AnimationState *animation, float time);
	void disableAnimations();
	bool initSkel;
	bool initHitBox;
	void createHitBox(std::string name);
	void drawHitBox(std::string name, btRigidBody *box);	
	void drawSkeleton();
	void createLine(std::string bone, int joint1, int joint2);
	void drawLine(std::string bone, int joint1, int joint2);
	void clearLine(std::string bone);


	bool getEnableKeyboard() { return mEnableKeyboard; }
	void setEnableKeyboard(bool enable) { mEnableKeyboard = enable; }
	void setAnimation(DynamicObject *p);

	// For testing purposes
//	Ogre::Overlay *setOverlay(Ogre::Overlay *o) { return overly = o;}

	bool start;

	DynamicObject *getDynamicObject() { return mPlayerObject; }
	DynamicObject *animObject;

	void setPosition (Ogre::Vector3 p);

    Ogre::SceneNode *getCameraNode () { return camNode; }
	Ogre::Vector3 getWorldPosition () { return mPlayerObject->mSceneNode->_getDerivedPosition (); }

protected:

	Ogre::SceneManager *mSceneManager;
	Ogre::SceneNode *camNode;

	InputHandler *mInputHandler;
	World *mWorld;
	Kinect *mKinect;
	DynamicObject *mPlayerObject;
	PhysicsManager *mPhysManager;
	Ogre::String meshName; 
	GameCamera *mCamera;

	bool canJump;
	bool isJumping;
	// For testing purposes 
//	Ogre::Overlay *overly; 
//	bool overlyBool; 

	// For kinect - Trying to follow format of RunnerTransfer for this 
	bool mEnableKeyboard;
	//bool mAutoCallibrate;
};

#endif