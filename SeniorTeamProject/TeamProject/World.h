#ifndef __World_h_
#define __World_h_

#include "PhysicsManager.h"
#include "DynamicObject.h"
#include "StaticScenery.h"
#include "GameLibrary.h"
#include "HUD.h"
#include "Console.h"


// Forward delcarations of Ogre classes.  Note the Ogre namespace!
namespace Ogre {
    class SceneNode;
    class SceneManager;
}

using namespace std;

// Forward delcarations of our own classes
class GameCamera;
class InputHandler;
class Player;
class Kinect;
class HUD;


class World
{
public:
	
	World(Ogre::SceneManager *sceneManager, InputHandler *input, Kinect *sensor, 
		GameCamera *gameCamera, GameLibrary *gameLib, Ogre::Root *mRoot, HUD *hud);

    // You'll want various methods to access & change your world here
    
    void Think(float time);
	void addCamera(GameCamera *c) { mCamera = c; }

	Ogre::SceneManager *SceneManager() { return mSceneManager; }

	PhysicsManager *physManager;

	GameLibrary *gameLibrary;
	
	GameCamera *mCamera;

	Ogre::SceneManager *mSceneManager;
	
	Player* mPlayer; 
	HUD *display;
	Stage* stage;	
	bool paused;

	void createWater();
	void doWaterStuff(float time);


protected:

	InputHandler *mInputHandler;

	Kinect *mKinect;

};

#endif