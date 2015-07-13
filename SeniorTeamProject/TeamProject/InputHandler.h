#ifndef __InputHandler_h_
#define __InputHandler_h_

#include <ois/ois.h>
#include <Ogre.h>
#include "Console.h"
#include "OgreRenderWindow.h"

class World;
class GameCamera;
class InputHandler;

class InputHandler : OIS::KeyListener  // : public OIS::MouseListener, public 
{

public:

	InputHandler(Ogre::RenderWindow* win, Console* console);
	~InputHandler();
	static InputHandler *getInstance();
	static void destroyInstance();

	bool IsKeyDown(OIS::KeyCode key);
	bool WasKeyDown(OIS::KeyCode key);

	void Think(float time);
	void initialize(Ogre::RenderWindow* win);
	void setEventCallback(OIS::KeyListener *keyListener);

protected:

	Console *mConsole;
	char mOldKeys[256];
	
	OIS::Keyboard *mCurrentKeyboard;
	OIS::Keyboard *mPreviousKeyboard;
	OIS::InputManager* mInputManager;
	Ogre::RenderWindow *mRenderWindow;

	virtual bool keyPressed(const OIS::KeyEvent& ke);
	virtual bool keyReleased(const OIS::KeyEvent& ke);

};

#endif