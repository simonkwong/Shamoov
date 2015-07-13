#ifndef __MainListener_h_
#define __MainListener_h_

#include "Ogre.h"
#include "OgreFrameListener.h"


// Forward declaration of classes
namespace Ogre
{
	class RenderWindow;
}

class InputHandler;
class World;
class GameCamera;
class Kinect;
class MenuManager;

class MainListener : public Ogre::FrameListener
{
public:
	MainListener(Ogre::RenderWindow *mainWindow, InputHandler *inputHandler, World *world, GameCamera *cam, Kinect *sensor);

	bool frameStarted(const Ogre::FrameEvent &evt);
	bool paused() { return mPaused; }
	void setPaused(bool paused) { mPaused = paused;}
    void quit() { mQuit = true;}

protected:
	InputHandler *mInputHandler;
	World *mWorld;
    GameCamera *mGameCamera;
	Kinect *mKinect;
	Ogre::RenderWindow *mRenderWindow;
	int x;
	bool mPaused;
	MenuManager *mMenus;
    bool mQuit;
};

#endif