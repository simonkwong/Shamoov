#ifndef __Camera_h_
#define __Camera_h_

namespace Ogre
{
    class Camera;
    class Vector3;
    class SceneNode;
    class SceneManager;
}

class World;
class Player;
class InputHandler;

class GameCamera
{

public:

	Ogre::Camera* mRenderCamera;
	GameCamera(Ogre::Camera* renderCamera, InputHandler* input, Ogre::SceneManager* sceneManager); 
	
	void setup();
	void Think(float time);
	void update(Player* player);
	void updatePosition(Ogre::Vector3 camP, Ogre::Vector3 tarP);

    // If you have a different cameras, you'll want some acessor methods here.
    // If your camera always stays still, you could remove this class entirely
protected:

	InputHandler* mInputHandler;
	Ogre::SceneNode* mCamNode;
	Ogre::SceneNode* mTargetNode;
	Ogre::SceneManager* mSceneManager; 
};

#endif