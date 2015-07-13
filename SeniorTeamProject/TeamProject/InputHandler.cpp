#include "InputHandler.h"
#include "Ogre.h"
#include "OgreStringConverter.h"
#include <OIS/OIS.h>

#include <stdio.h>


InputHandler::InputHandler(Ogre::RenderWindow *renderWindow, Console* console) : 
	 mRenderWindow(renderWindow), mConsole(console)
{
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	renderWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mCurrentKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true /* not buffered */ ));
	mCurrentKeyboard->setEventCallback(this);
}

void InputHandler::setEventCallback(OIS::KeyListener *keyListener)
{
	mCurrentKeyboard->setEventCallback(keyListener);
}


bool
InputHandler::IsKeyDown(OIS::KeyCode key)
{
	// if the console is visible ignore all keyboard input
	if(!mConsole->isVisible()) {
		return mCurrentKeyboard->isKeyDown(key);
	} else {
		return false;
	}
}

bool
InputHandler::WasKeyDown(OIS::KeyCode key)
{
	return mOldKeys[key] != '\0';
}

void 
InputHandler::Think(float time)
{
	mCurrentKeyboard->copyKeyStates(mOldKeys);
	mCurrentKeyboard->capture();
}


InputHandler::~InputHandler()
{
	mInputManager->destroyInputObject(mCurrentKeyboard);
}

bool InputHandler::keyPressed(const OIS::KeyEvent& ke)  {
	mConsole->onKeyPressed(ke);
	if (ke.key == OIS::KC_TAB) {
		if(mConsole->isVisible()) {
			mConsole->setVisible(false);
		} else {
			mConsole->setVisible(true);
		}
	}
	return true;
}

bool InputHandler::keyReleased(const OIS::KeyEvent& ke) 
{ 
  return true; 
}