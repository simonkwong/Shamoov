#ifndef __TeamProject_h_
#define __TeamProject_h_

// Including header files within header files is frowned upon,
// but this one is requried for Ogre Strings.
#include "OgrePrerequisites.h"
#include "OgreWindowEventUtilities.h"
#include "GameLibrary.h"


namespace Ogre {
    class Root;
    class RenderWindow;
    class Camera;
    class SceneManager;
	class OverlaySystem;
}
class InputHandler;
class World;
class GameCamera;
class MainListener;
class Logger;
class Kinect;
class HUD; 

class TeamProject :  public Ogre::WindowEventListener
{
public:
    TeamProject();
    ~TeamProject();

    // Do all the application setup
    bool setup(void);

    // Start runn
    void go(void);

protected:

    ///////////////////////////////////////////////////
    /// The following methods are all called by the public
    ///   setup method, to handle various initialization tasks
    //////////////////////////////////////////////////

    //  Load all the requrired resources (Mostly reading the file paths
    //  of various resources from the .cfg files)
    void setupResources(void);

    // Invoke the startup window for all of the Ogre settings (resolution, etc)
    bool configure(void);

    // Create all of the required classes and do setup for all non-rendering tasks
    void createScene(void);

    // Free up all memory & resources created in createScene
    void destroyScene(void);

	void setFromConfigString(std::string, int val = 0);
    void setSingleConfig(std::string key, std::string value, int val = 0);
	// Create the rendering camera (separate from the game camera -- the game camera
    //   contains the logic of how the camera should be moved, the rendering camera
    //   is used by the rendering system to render the scene.  So, the game camera 
    //   decides where the camera should be, and then makes calls to the rendering camera
    //   to move the camera
	void createCamera(void);

	void createViewports(void);

    // The FrameListener is what receives callbacks from the rendering loop to handle 
    //  game logic
	void createFrameListener(void);

	// Starts the game 
	void startGame(void);

	// Ends the game 
	void endGame(void);

	// sets up menus 
	void setupMenus(bool loginRequired);
	
	void readConfigStr(int val = 0);

	MainListener *mFrameListener;

	InputHandler *mInputHandler;
	World *mWorld;
    GameCamera *mGameCamera;
	Kinect *mKinect;
	HUD *display;
	Logger *mLogger;
	
    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcePath;
	Ogre::OverlaySystem *mOverlaySystem;

};

#endif // #ifndef __TeamProject_h_