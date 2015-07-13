#ifndef CONSOLE_H
#define CONSOLE_H
#include <OgreFrameListener.h>
#include "OgreOverlayManager.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayContainer.h"
#include <Ogre.h>
#include <OIS/OIS.h>
#include <list>
#include <vector>
#include "OgreOverlay.h"
#include "World.h"
#include "Camera.h"
#include "Player.h"

class World;

class Console: Ogre::FrameListener
{

public:
	Console(Ogre::Root* mRoot, World* mWorld);
    ~Console();

	World* mWorld;

    void setVisible(bool visible);
    bool isVisible() {return visible;}
 

	void print(const Ogre::String &text);


	virtual bool frameStarted(const Ogre::FrameEvent &evt);
    virtual bool frameEnded(const Ogre::FrameEvent &evt);
 
    void onKeyPressed(const OIS::KeyEvent &arg);
 
    void addCommand(const Ogre::String &command, void (*func)(Console* console, std::vector<Ogre::String>* args));
    void removeCommand(const Ogre::String &command);
	
    bool visible;
    bool initialized;
    Ogre::Root* mRoot;
    Ogre::SceneManager* scene;
    Ogre::Rectangle2D* rect;
    Ogre::SceneNode* node;
    Ogre::OverlayElement* textbox;
    Ogre::Overlay* overlay;
    float height;
    bool update_overlay;
    int start_line;
    std::list<Ogre::String> lines; /* All the lines of text in the console */
	Ogre::String prompt;
	std::map<Ogre::String,void (*)(Console* console, std::vector<Ogre::String>* args)> commands;
	bool isInPlacementMode;
	std::string statSceneNameToPlace;
	Ogre::Vector3 toPlacePos;
	Ogre::SceneNode* placementNode;
	void placeStaticScenery();
};


/* Command Functions */
void help(Console* console, std::vector<Ogre::String>* args);
void ls(Console* console, std::vector<Ogre::String>* args);
void add(Console* console, std::vector<Ogre::String>* args);
void lsd(Console* console, std::vector<Ogre::String>* args);

#endif