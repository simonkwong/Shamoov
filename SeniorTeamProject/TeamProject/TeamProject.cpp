#include "TeamProject.h"
#include "World.h"
#include "InputHandler.h"
#include "MainListener.h"
#include "Camera.h"
#include "Kinect.h"
#include "HUD.h"
#include "Menu.h"
#include "Logger.h"
#include "JsonUtils.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "Sound.h"

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreFontManager.h"
#include "OgreOverlaySystem.h"



TeamProject::TeamProject()
{
    mFrameListener = 0;
    mRoot = 0;
    // Provide a nice cross platform solution for locating the configuration files
    // On windows files are searched for in the current working directory, on OS X however
    // you must provide the full path, the helper function macBundlePath does this for us.
	//  (Note:  This is not fully tested under IOS)
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    mResourcePath = macBundlePath() + "/Contents/Resources/";
#else
    mResourcePath = "";
#endif
}

/// Standard destructor
TeamProject::~TeamProject()
{
    if (mFrameListener)
        delete mFrameListener;
    if (mRoot)
        delete mRoot;
}



void
TeamProject::createCamera()
{
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setPosition(Ogre::Vector3(0,0,0));
	mCamera->lookAt(Ogre::Vector3(0,0,0));
}



// We will create a single frame listener, to handle our main event loop.  While we could
// add as many frame listeners as we want (and let the main rendering loop call them),
// we would not have as much control over the order that the listeners are called --
//  so we will just have one listener and handle everything ourselves.
void 
TeamProject::createFrameListener(void)
{
	mFrameListener = new MainListener(mWindow, mInputHandler, mWorld, mGameCamera, mKinect);
	mRoot->addFrameListener(mFrameListener);
}

// We will have a single viewport.  If you wanted splitscreen, or a rear-view mirror, then
// you may want multiple viewports.
void 
TeamProject::createViewports(void)
{
	    // Create one viewport, entire window
        Ogre::Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(Ogre::ColourValue(0,255,0));
        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));    
}

// Here is where we set up all of the non-rendering stuff (our world, various managers, etc)
void 
TeamProject::createScene() 
{
	Console *console = new Console(mRoot, this->mWorld);
	console->setVisible(false);

    mInputHandler = new InputHandler(mWindow, console);
	
	MenuManager* menuManager = MenuManager::getInstance();
	menuManager->mConsole = console;
	mInputHandler->setEventCallback(menuManager);

	// If a class needs access to another class, you can pass in a pointer in the constructor
	//   or, if you need circular accesses (player needs access to the world, and the world needs
	//   access to the player), you can add the pointers later.  Here's an example of both ways
	//   of doing it, giving the world access to the camera and the input handler.
	
	mGameCamera = new GameCamera(mCamera, mInputHandler, mSceneMgr);   
	
	mKinect = new Kinect();
	mKinect->initSensor();
	//mKinect->StartSession();
	display = new HUD();
	mWorld = new World(mSceneMgr, mInputHandler, mKinect, mGameCamera, new GameLibrary(mSceneMgr), this->mRoot, display);
	
	// so ugly cringe coding omg im sorry
	console->mWorld = mWorld;
	// create placement Node
	console->placementNode = mWorld->mSceneManager->getRootSceneNode()->createChildSceneNode();
	console->placementNode->setVisible(false);

	mLogger = new Logger();

	// Display whether user is logged in [found on the upper left corner]  
	Ogre::OverlayManager::getSingleton().getByName("Login/Failure")->show();

}

void TeamProject::setFromConfigString(std::string configString, int val)
{

	///TODO:  This parsing of JSON dictionaries really needs to be refactored ..

	std::size_t braceIndex = configString.find_first_of('{');
	if (braceIndex == std::string::npos)
	{
		return;
	}

	std::string remainder = configString.substr(braceIndex+1);

	std::size_t nextIndex = remainder.find_first_not_of("\t \n");
	while (nextIndex != std::string::npos && remainder[nextIndex] != '}')
	{
		std::string nextKey = JSON_UTIL::stripQuotes(JSON_UTIL::firstItem(remainder));

		remainder = JSON_UTIL::removeFirstitem(remainder);
	
		size_t colonIndex = remainder.find_first_of(":");
		if (colonIndex == std::string::npos)
		{
			// ERROR!
			nextIndex = std::string::npos;
			break;
		}
		remainder = remainder.substr(colonIndex+1);
		std::string value = JSON_UTIL::firstItem(remainder);
		remainder = JSON_UTIL::removeFirstitem(remainder);
		setSingleConfig(nextKey, value, val);

		nextIndex = remainder.find_first_not_of("\t \n");
		if (nextIndex != std::string::npos && remainder[nextIndex] == ',')
			nextIndex++;
		remainder = remainder.substr(nextIndex);
		nextIndex = remainder.find_first_not_of("\t \n");
	}
}

void TeamProject::setSingleConfig(std::string key, std::string value, int val)
{
	key = JSON_UTIL::stripQuotes(key);
	/*if (key == "menus")
	{
		MenuManager::getInstance()->setMenuConfig(value);
	}
	else if (key == "achievements")
	{
		mAchievements[player]->setCompletedAchievements(value);
	}
	else if (key == "coins")
	{
		mPlayer[player]->setTotalCoins(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
	else if (key == "lifetimeCoins")
	{
		mPlayer[player]->setLifetimeCoins(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
	else if (key == "totalMeters")
	{
		mPlayer[player]->setTotalMeters((float)atof(JSON_UTIL::stripQuotes(value).c_str()));

	}
	else if (key == "armor")
	{
		mPlayer[player]->setInitialArmor(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}

	else if (key == "boostFrequency")
	{
		mWorld->setBoostFreq(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
		else if (key == "boostDuration")
	{
		mPlayer[player]->setBoostDuration(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}

		else if (key == "shieldFrequency")
	{
		mWorld->setShieldFreq(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
		else if (key == "shieldDuration")
	{
		mPlayer[player]->setShieldDuration(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
	    else if (key == "magnetFrequency")
	{
		mWorld->setShieldFreq(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}
		else if (key == "magnetDuration")
	{
		mPlayer[player]->setShieldDuration(atoi(JSON_UTIL::stripQuotes(value).c_str()));
	}*/
}


void TeamProject::readConfigStr(int val)
{
	MenuManager *menus = MenuManager::getInstance();

	std::string config = mLogger->getProfileData();
	
	if (config.size() > 0)
	{
		setFromConfigString(config, val);
	}
}

void
TeamProject::setupMenus(bool loginRequired)
{
	MenuManager *menus = MenuManager::getInstance();
    MainListener *l = mFrameListener;
	HUD *h = display;
    World *w = mWorld;
    Kinect *k = mKinect;
	Logger *lm = mLogger;
	SoundBank *sb = SoundBank::getInstance();

    Menu *mainMenu = new Menu("Main Menu", "main", 0.05f, 0.1f, 0.08f);

    Menu *options = new Menu("Options", "options", 0.05f, 0.1f, 0.1f, mainMenu);
    Menu *controlOptions = new Menu("Control Options", "controloptions", 0.05f, 0.1f, 0.07f, options);
	Menu *instructions = new Menu("Instructions", "instructions", 0.05f, 0.1f, 0.1f, mainMenu);
    //Menu *gameplayOptions = new Menu("Gameplay Options", "gameplayoptions", 0.05f, 0.05f, 0.07f, options);
    Menu *soundOptions = new Menu("Sound Options", "soundOptions", 0.05f, 0.1f,0.1f, options);
    Menu *advancedOptions = new Menu("Advanced Options", "advancedOptions", 0.05f, 0.1f, 0.08f, options);
    Menu *login = new Menu("Login", "login", 0.05f, 0.1f,0.1f, mainMenu);

	Menu *pauseMenu = new Menu("Pause Menu", "pause", 0.05f, 0.1f);
    Menu *confirmMenu = new Menu("Confirm Profile Reset", "profleReset", 0.1f, 0.1f, 0.1f, advancedOptions);
	Menu *endGameMenu = new Menu("Game Over!", "gameOver", 0.1f, 0.1f, 0.1f, NULL);

	menus->addMenu(mainMenu);
    menus->addMenu(options);
    menus->addMenu(pauseMenu);
	//menus->addMenu(gameplayOptions);
	menus->addMenu(controlOptions);
	
	menus->addMenu(soundOptions);
	menus->addMenu(advancedOptions);
	menus->addMenu(instructions);
	menus->addMenu(login);
	menus->addMenu(endGameMenu);
	menus->addMenu(confirmMenu);

	/////////////////////////////////////////////////
	// Instructions Menu 
	//////////////////////////////////////////////////
	instructions->AddSelectElement("Return to Main Menu", [instructions, mainMenu, h]() {instructions->disable(); 
		mainMenu->enable(); h->displayInstructions(false);});

	/////////////////////////////////////////////////
	// Login Menu 
	//////////////////////////////////////////////////

	login->AddChooseString("Username",[lm](Ogre::String s) {lm->changeUsername(s); },"",15,false);
	login->AddChooseString("Password",[lm, this](Ogre::String s) {this->setFromConfigString(lm->changePassword(s));},"",15,true);
	login->AddSelectElement("Return to Main Menu", [login, mainMenu]() {login->disable(); mainMenu->enable();});

	/////////////////////////////////////////////////
	// Options Menu 
	//////////////////////////////////////////////////

    options->AddSelectElement("Control Options", [options, controlOptions]() {options->disable(); controlOptions->enable();});
    //options->AddSelectElement("Gameplay Options", [options, gameplayOptions]() {options->disable(); gameplayOptions->enable();});
    options->AddSelectElement("Sound Options", [options, soundOptions]() {options->disable(); soundOptions->enable();});
    options->AddSelectElement("Advanced Options", [options, advancedOptions]() {options->disable(); advancedOptions->enable();});
	options->AddSelectElement("Return to Main Menu", [options, mainMenu]() {options->disable(); mainMenu->enable();});
	
	
	/////////////////////////////////////////////////
	// Options Submenu:  Controls 
	//////////////////////////////////////////////////

    //controlOptions->AddChooseBool("Callibrate Kinect Every Game", [k](bool x) {k->setAutoCallibrate(x); }, k->getAutoCallibrate(), true);
    //controlOptions->AddChooseFloat("Kinect Sensitivity Left / Right", [p](float x) {p->setKinectSentitivityLR(x); }, 0.7f, 1.5f, 1.f, 0.1f, true);
    //controlOptions->AddChooseFloat("Kinect Sensitivity Front / Back", [p](float x) {p->setKinectSentitivityFB(x); }, 0.7f, 1.5f, 1.f, 0.1f, true);
    controlOptions->AddSelectElement("Callibrate Kinect Now", [controlOptions, k]() {controlOptions->disable(); k->callibrate(45.0f, [controlOptions]() {controlOptions->enable();});});
    //controlOptions->AddChooseBool("Invert Front/Back Controls", [p](bool x) {p->setInvertControls(x); }, p->getInvertControls(), true);
	controlOptions->AddChooseBool("Enable Kinect", [k](bool x) { k->setEnableKinect(x);  if (!x) k->setAutoCallibrate(false); }, k->getEnableKinect(), true);
	//controlOptions->AddChooseBool("Enable Keyboard", [p](bool x) { p->setEnableKeyboard(x);}, p->getEnableKeyboard(), true);

   controlOptions->AddSelectElement("Return to Options Menu", [controlOptions,options]() {controlOptions->disable(); options->enable();});

	/////////////////////////////////////////////////
	// Options Submenu:  Sounds 
	//////////////////////////////////////////////////

    soundOptions->AddChooseBool("Enable Sound", [sb](bool x) {sb->setEnableSound(x); }, sb->getEnableSound(), true);
	soundOptions->AddChooseInt("Volume", [sb](int x) {sb->setVolume(x); }, 0, 128, sb->getVolume(), 5, true);

	/*std::vector<Ogre::String> namesSoundType;
	std::vector<std::function<void()>> callbacksSoundType;
	namesSoundType.push_back("Realistic (blades)");
	callbacksSoundType.push_back([sb]() {sb->setCurrentIndex(0); });

	namesSoundType.push_back("Representational (tones)");
	callbacksSoundType.push_back([sb]() {sb->setCurrentIndex(1); });

	soundOptions->AddChooseEnum("Sound Type",namesSoundType,callbacksSoundType,0, true);	*/
	soundOptions->AddSelectElement("Return to Options Menu", [soundOptions,options]() {soundOptions->disable(); options->enable();});

	/////////////////////////////////////////////////
	// Main Menu 
	//////////////////////////////////////////////////


	mainMenu->AddSelectElement("Start Standard Game", [mainMenu,this]() { mainMenu->disable(); this->startGame(); });
	mainMenu->AddSelectElement("Login", [mainMenu, login]() {mainMenu->disable(); login->enable();});
	mainMenu->AddSelectElement("Options", [options, mainMenu]() {options->enable(); mainMenu->disable();});
	mainMenu->AddSelectElement("Instructions", [instructions, mainMenu, h]() {instructions->enable(); 
	mainMenu->disable(); h->displayInstructions(true);});
	mainMenu->AddSelectElement("Quit", [l]() {l->quit();});

	/////////////////////////////////////////////////
	// Pause Menu 
	//////////////////////////////////////////////////


    pauseMenu->AddSelectElement("Continue", [pauseMenu, w]() {pauseMenu->disable(); w->paused = false; });
    //pauseMenu->AddSelectElement("End Game (Return to Main Menu)", [pauseMenu,mainMenu, p, w, h, this]() {this->endGame(), h->showHUDElements(false); pauseMenu->disable();mainMenu->enable(); p->setPaused(true); });
    pauseMenu->AddSelectElement("Quit (Close Program)", [this, l]() {l->quit();}); //{this->writeConfigStr();l->quit();});

	/////////////////////////////////////////////////
	// End Game / Ghost Menu 
	////////////////////////////////////////////////


	//endGameMenu->AddSelectElement("Replay Against Ghost", [this, endGameMenu]() {endGameMenu->disable(); this->replayGhost();});
	//endGameMenu->AddSelectElement("Save Ghost", [ghost, awkGhostSave, endGameMenu]() {endGameMenu->disable(); ghost->writeFile(); awkGhostSave->enable();  });
    //endGameMenu->AddSelectElement("Return to Main Menu", [endGameMenu,mainMenu, p, w, h, this]() {this->endGame(), h->showHUDElements(false); endGameMenu->disable();mainMenu->enable(); p->setPaused(true); });

	//awkGhostSave->AddSelectElement("OK", [endGameMenu, awkGhostSave]() {endGameMenu->enable(); awkGhostSave->disable();});




	/////////////////////////////////////////////////
	// Options Submenu:  Advanced 
	//////////////////////////////////////////////////

    advancedOptions->AddSelectElement("Get Profile from Server", [this]() {this->readConfigStr();});
    advancedOptions->AddSelectElement("Reset Profile", [advancedOptions, confirmMenu]() {advancedOptions->disable();confirmMenu->enable();});
	advancedOptions->AddSelectElement("Return to Options Menu", [advancedOptions, options]() {advancedOptions->disable(); options->enable();});
    confirmMenu->AddSelectElement("Reset Profile (Cannot be undone!)", [this,advancedOptions, confirmMenu, menus]() {menus->resetMenus();
	this->setupMenus(false);});
    confirmMenu->AddSelectElement("Cancel Profile Reset", [advancedOptions, confirmMenu]() {advancedOptions->enable();confirmMenu->disable();});

	/////////////////////////////////////////////////
	// End of Menu Code
	//////////////////////////////////////////////////


	if (loginRequired)
	{
		login->enable();
	}
	else
	{
		mainMenu->enable();
	}
}


#pragma region To Include Later 
void 
TeamProject::startGame()
{
	mKinect->StartSession();
	mWorld->mPlayer->start = true;
	display->displayScore(true);
	display->displayMenuBG(false);
}

void 
TeamProject::endGame()
{
	mKinect->EndSession();
}
#pragma endregion 

bool 
TeamProject::setup(void)
{
    Ogre::String pluginsPath;
	Ogre::String configPath;
    // only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB

#if OGRE_DEBUG_MODE == 1 && (OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS)
	// add _d suffix to config files
	pluginsPath = mResourcePath + "plugins_d.cfg";
	configPath = mResourcePath + "ogre_d.cfg";
#else
	pluginsPath = mResourcePath + "plugins.cfg";
	configPath = mResourcePath + "ogre.cfg";

#endif


#endif
	
    mRoot = new Ogre::Root(pluginsPath, 
        configPath, mResourcePath + "Ogre.log");

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    // Create the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "SMInstance");
	createCamera();    
    createViewports();


    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);


	mOverlaySystem = new Ogre::OverlaySystem();
	mSceneMgr->addRenderQueueListener(mOverlaySystem);

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    createScene();


    // When we start the renderer, it will run forever, making callbacks every
    // frame to the frame listeners.  These listeners are where all of the non-
    // rendering work is done.  
    createFrameListener();

	setupMenus(true);

    return true;

}
/** Configures the application - returns false if the user chooses to abandon configuration. */
bool 
TeamProject::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true);
        return true;
    }
    else
    {
        return false;
    }
}

void
TeamProject::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;

	#if OGRE_DEBUG_MODE == 1 && (OGRE_PLATFORM != OGRE_PLATFORM_APPLE && OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS)
			// add _d suffix to config files
    cf.load(mResourcePath + "resources_d.cfg");
#else
	cf.load(mResourcePath + "resources.cfg");
#endif
		

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            // OS X does not set the working directory relative to the app,
            // In order to make things portable on OS X we need to provide
            // the loading with it's own bundle path location
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                String(macBundlePath() + "/" + archName), typeName, secName);
#else
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
#endif
        }
    }
}

void
TeamProject::destroyScene()
{
    delete mWorld;
    delete mGameCamera;
    delete mInputHandler;
}

void 
TeamProject::go(void)
{
    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}


