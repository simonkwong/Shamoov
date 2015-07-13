#include "Console.h"

 
#define CONSOLE_LINE_LENGTH 85
#define CONSOLE_LINE_COUNT 15

 
Console::Console(Ogre::Root *mRoot, World* mWorld) {
 
	this->mWorld = mWorld;
	start_line=0;
    this->mRoot=mRoot;
    scene=mRoot->getSceneManagerIterator().getNext();
    mRoot->addFrameListener(this);
	isInPlacementMode = false;



    height=1;
 
    // Create background rectangle covering the whole screen
    rect = new Ogre::Rectangle2D(true);
    rect->setCorners(-1, 1, 1, 1-height);
    rect->setMaterial("console/background");
    rect->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    rect->setBoundingBox(Ogre::AxisAlignedBox(-100000.0*Ogre::Vector3::UNIT_SCALE, 100000.0*Ogre::Vector3::UNIT_SCALE));
    node = scene->getRootSceneNode()->createChildSceneNode("#Console");
    node->attachObject(rect);
 
    textbox = Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea","ConsoleText");
    textbox->setCaption("hello");
    textbox->setMetricsMode(Ogre::GMM_RELATIVE);
    textbox->setPosition(0,0);
    textbox->setParameter("font_name","Console");
    textbox->setParameter("colour_top","1 1 1");
    textbox->setParameter("colour_bottom","1 1 1");
    textbox->setParameter("char_height","0.03");
 
    overlay=Ogre::OverlayManager::getSingleton().create("Console");   
    overlay->add2D((Ogre::OverlayContainer*)textbox);
    overlay->show();
	
	this->addCommand("ls", &ls);
	this->addCommand("help", &help);	
	this->addCommand("add", &add);
	this->addCommand("lsd", &lsd);

}

/* Console Commands */
/* List all of the commands in the console */
void help(Console *console, std::vector<Ogre::String> *args) {
	for(int i = 1; i < args->size(); i++) {
		console->print("Here is a list of commands " + args->at(i));
	}
}

/* List all of the objects in the world by name */
void ls(Console *console, std::vector<Ogre::String> *args) {
	console->print("The world contains: ");
	
	Ogre::SceneManager::MovableObjectIterator iterator = console->scene->getMovableObjectIterator("Entity");
	while(iterator.hasMoreElements())
	{
		Ogre::Entity* e = static_cast<Ogre::Entity*>(iterator.getNext());
		console->print(e->getName());
	}
}

/*
	Get information about the objects being watched by the physmanager.
*/
void lsd(Console *console, std::vector<Ogre::String> *args) {
	console->print("Physmanager is watching " + std::to_string(console->mWorld->physManager->physObjects.size()) + " items");
}

/*
	Loads and object from the game library and adds it to the game.
	at the current camera position.
*/
void add(Console *console, std::vector<Ogre::String> *args) {
	
	if (args->size() != 3) {
		if(args->size() >= 2) {
			if ((*args)[1].compare("help") == 0) {
				console->print("the only thing add supports right now is dynamic objects!");
				console->print("use -d to specify a dynamic object");
				console->print("For example: add -d TeaPot");
			} else {
				console->print("usage: add <object type> <object name>");
				console->print("for more info type: add help");
			}
		} else {
			console->print("usage: add <object type> <object name>");
			console->print("for more info type: add help");
		}
	} else {
		if ((*args)[1].compare("-d") == 0) {
			// adding a dynamic object
			DynamicObject* tempObj = console->mWorld->gameLibrary->getDynamicObject((*args)[2]);

			if (tempObj != NULL) {
				console->print("Loaded!");
				tempObj->addToBullet(console->mWorld->physManager);
				console->print("Added to Bullet!");
				Ogre::Vector3 camPos = console->mWorld->mCamera->mRenderCamera->getPosition();
				tempObj->setPosition(camPos + Ogre::Vector3(0, 0, 50));
				tempObj->addToOgreScene(console->mWorld->mSceneManager);
				console->print("Added to Ogre!");
			} else {
				console->print("No DynamicObject with name \"" + (*args)[2] + "\" was found");
			} 
		} else if((*args)[1].compare("-s") == 0) {

			// adding a dynamic object
			StaticScenery* tempObj = console->mWorld->gameLibrary->getStaticScenery((*args)[2], console->mWorld->mCamera->mRenderCamera->getPosition() + Ogre::Vector3(0, -20, -30), Ogre::Quaternion::IDENTITY);

			// adding a StaticScenery object
			Ogre::Vector3 objectPos = console->mWorld->mPlayer->getWorldPosition();
			
			
			/* enter placement mode */
			console->placementNode->detachAllObjects();
			console->placementNode->setPosition(objectPos);
			// get the entity
			Ogre::Entity* tempEntity = console->mWorld->mSceneManager->createEntity((*args)[2] + ".MESH.mesh");
			console->placementNode->attachObject(tempEntity);
			console->placementNode->setVisible(true);
			console->statSceneNameToPlace = (*args)[2];
			console->setVisible(false);
			console->mWorld->mPlayer->setEnableKeyboard(false);
			console->isInPlacementMode = true;


		} else {
			console->print("object type \"" + (*args)[1] + "\" not recognized");
		}
	}		
}


 
 Console::~Console() {
    if(!initialized)
       return;
    delete rect;
    delete node;
    delete textbox;
    delete overlay;
 }


 void Console::onKeyPressed(const OIS::KeyEvent &arg){
    if(!visible)
       return;
	if (arg.key == OIS::KC_RETURN && prompt.length() != 0)
    {
       //split the parameter list
       const char *str=prompt.c_str();
	   print(str);
       std::vector<Ogre::String> params;
       Ogre::String param="";
       for(int c=0;c<prompt.length();c++){
          if(str[c]==' '){
             if(param.length())
                params.push_back(param);
             param="";
          }
          else
             param+=str[c];
       }
       if (param.length()) {
          params.push_back(param);
	   }

		bool commandFound = false;

		// try to execute the command
		std::map<Ogre::String, void (*)(Console *console, std::vector<Ogre::String> *args)>::iterator i;
		for (i=commands.begin();i!=commands.end();i++) {
			if ((*i).first==params[0]) {
				std::vector<Ogre::String> *args = new std::vector<Ogre::String>(params.begin(), params.end());
				(* ((*i).second)) (this, args);
				commandFound = true;
				break;
			}
		}
		
		if (commandFound == false) {
			print("Command \"" + params[0] + "\" not found");
		}

		prompt = "";

    }
    if (arg.key == OIS::KC_BACK)
       prompt=prompt.substr(0,prompt.length()-1);
    if (arg.key == OIS::KC_PGUP)
    {
       if(start_line>0)
          start_line--;
    }
    if (arg.key == OIS::KC_PGDOWN)
    {
       if(start_line<lines.size())
          start_line++;
    }
    else
    {
		if(arg.key != OIS::KC_LSHIFT && arg.key != OIS::KC_RSHIFT &&
		   arg.key != OIS::KC_PGUP & arg.key != OIS::KC_LEFT && 
		   arg.key != OIS::KC_RIGHT && arg.key != OIS::KC_UP &&
		   arg.key != OIS::KC_DOWN) {
			char legalchars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+!\"#%&/()=?[]\\*-_.:,; ";
			for(int c=0;c<sizeof(legalchars);c++){
				if(legalchars[c]==arg.text) {
					prompt+=arg.text;
					break;
				}
			}
		}
    }
    update_overlay=true;
 }



 bool Console::frameStarted(const Ogre::FrameEvent &evt){
	 if(visible&&height<1){
       height+=evt.timeSinceLastFrame*2;
       textbox->show();
       if(height>=1){
          height=1;
       }
    }
    else if(!visible&&height>0){
       height-=evt.timeSinceLastFrame*2;
       if(height<=0){
          height=0;
          textbox->hide();
       }
    }
 
    textbox->setPosition(0,(height-1)*0.5);
    rect->setCorners(-1,1+height,1,1-height);
 
    if(update_overlay){
       Ogre::String text;
       std::list<Ogre::String>::iterator i,start,end;
 
       //make sure is in range
       if(start_line>lines.size())
          start_line=lines.size();
 
       int lcount=0;
       start=lines.begin();
       for(int c=0;c<start_line;c++)
          start++;
       
	   end=start;
       for(int c=0;c<CONSOLE_LINE_COUNT;c++){
          if(end==lines.end())
             break;
          end++;
       }
       for(i=start;i!=end;i++)
          text+=(*i)+"\n";
 
       //add the prompt
       text+=">"+prompt;
 
       textbox->setCaption(text);
       update_overlay=false;
    }
    return true;
 }
 
 void Console::print(const Ogre::String &text){
    //subdivide it into lines
    const char *str=text.c_str();
    int start=0,count=0;
    int len=text.length();
    Ogre::String line;
    for(int c=0;c<len;c++){
       if(str[c]=='\n'||line.length()>=CONSOLE_LINE_LENGTH){
          lines.push_back(line);
          line="";
       }
       if(str[c]!='\n')
          line+=str[c];
    }
    if(line.length())
       lines.push_back(line);
    if(lines.size()>CONSOLE_LINE_COUNT)
       start_line=lines.size()-CONSOLE_LINE_COUNT;
    else
       start_line=0;
    update_overlay=true;
 }
 
 bool Console::frameEnded(const Ogre::FrameEvent &evt){
 
    return true;
 }
 
 void Console::setVisible(bool visible){
    this->visible=visible;
 }
 
 void Console::addCommand(const Ogre::String &command, void (*func)(Console *console, std::vector<Ogre::String> *args)){
	 void (*temp)(Console *console, std::vector<Ogre::String> *args) = func;
	 commands[command] = temp;
 }
 
 void Console::removeCommand(const Ogre::String &command){
    commands.erase(commands.find(command));
 }

 void Console::placeStaticScenery() {
	StaticScenery* tempObj = this->mWorld->gameLibrary->getStaticScenery(statSceneNameToPlace, placementNode->getPosition(), Ogre::Quaternion::IDENTITY);
	if (tempObj != NULL) {
		this->print("Loaded!");
		tempObj->addToBullet(this->mWorld->physManager);
		this->print("Added to Bullet!");
		tempObj->addToOgreScene(this->mWorld->mSceneManager);
		this->print("Added to Ogre!");
	} else {
		this->print("No StaticScenery with name \"" + statSceneNameToPlace + "\" was found");
	}


	// exit placement mode
	this->placementNode->detachAllObjects();
	this->placementNode->setVisible(false);
	this->mWorld->mPlayer->setEnableKeyboard(true);
	this->isInPlacementMode = false;
 }
 
