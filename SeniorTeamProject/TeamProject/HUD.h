#ifndef __HUD_h_
#define __HUD_h_

#include "OgreOverlay.h"
#include "OgreFontManager.h"
#include "OgreOverlayManager.h"
#include "OgreTextAreaOverlayElement.h"

using namespace std;

class HUD
{

public:

	HUD();
	~HUD();

	int displayScore() { return score; }

	void incrementScore();
	void setInstructions();
	void displayHint(bool display);
	void displayScore(bool display);
	void displayEnding(bool display);
	void displayMenuBG(bool display);
	void displayInstructions(bool display);

protected: 

	Ogre::Overlay *hintsOverlay;
	Ogre::Overlay *scoreOverlay;
	Ogre::Overlay *endingOverlay;
	Ogre::Overlay *menuBgOverlay;
	Ogre::Overlay *instructionsOverlay; 

	Ogre::OverlayElement *inText;
	Ogre::OverlayElement *hintText;
	Ogre::OverlayElement *scoreText;
	Ogre::OverlayElement *endingText; 

	int score; 
	void setScore();

};

#endif