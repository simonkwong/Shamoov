#include "GameLibrary.h"

/* A library of content that can be put into the game.
 *
 *	Glossary:
 *		Part 1: Loading things into the library from JSON files
 *		Part 2: Getting things from the library into the game
 *
 * ############################################################
 * # Part 1: Loading things in to the library from JSON files #
 * ############################################################

 * Data in the library is loaded from json files which specify class and how 
 * configurations for what you are putting in the library.
 *
 * For example:
 * To make a coin you could make the following Coin.json file:
 *
 * __________________________________________________________
 *	{
 *		"DynamicObject" : 
 *		{
 *			"meshName" : "Coin.MESH.mesh",
 *			"mass" : 1,
 *			"collisionShape" : "btBoxShape",
 *			"collisionShapeSize" : [1,1,1],
 *			"restitution" : 1
 *		}
 *	}
 * ____________________________________________________________
 *
 * The class you use depends on what you want the object to do, for the coin I
 * chose DynamicObject because I wanted it to have collision and physics.
 *
 *
 *
 * Rules for writing .json library files:
 *
 *		1. Use this format: { {"CLASSNAME" : <PROPERTIES> } }
 *		The CLASSNAME let's the GameLibrary loader know how to parse the PROPERTIES
 *		part. You can define the properties however you like to describe a class.
 *
 *		2. The filename is important!
 *		   The file name is used for lazy loading of object.
 *         Rather than having a seperate mapping of object names to the files that 
 *		   describe them, the file name simply is the same as the object name.
 *   	   For example: if the program calls getDynamicObject("Coin") and it has not 
 *		   been loaded into the Library yet, Library will look for and load a file 
 *		   called Coin.json.
 *
 *
 * #########################################################
 * # Part 2: Getting things from the library into the game #
 * #########################################################
 *
 * To load an object from the library simply use the method corresponding to
 * the object type you want to load. For example, loading a dynamic object
 * might looks like this:
 *
 * DynamicObject *d = gameLibrary->getDynamicObject("Box");
 *
 * To actually put the object into the game you must add it to
 * the bullet and ogre world, for example:
 *
 * d->addToOgreScene(mSceneManager);
 * d->addToBullet(physManager);




 Stage files contain references to things in the game library.

 It also says where in the world they should be.

 Position shouldn't be in a prefab file because 
 position describes its space in a stage.
 Thus position should be given in a stage file.



 */


GameLibrary::GameLibrary(Ogre::SceneManager *sceneManager) {
	this->mSceneManager = sceneManager;
}


/*
	Retrieves a DynamicObject from the game library.
	Returns null if no object was found that matches name.
*/
DynamicObject * GameLibrary::getDynamicObject(string name) {
	// see if an instance of the object exists in dynamicObjects map.
	// if not load it in from memory, create it, and put it in the map.

	unordered_map<string, DynamicObject*> ::iterator it = dynamicObjects.find(name);

	DynamicObject *dynObj;
 	if(it != dynamicObjects.end())
	{
		//element found;
		dynObj = it->second;

		// create a clone of it.
		return dynObj->clone(this->mSceneManager);

	} else {
		// element was not found.
		// load it in and create instance 

		std::string fileName = "../TeamProject/GameData/DynamicObjects/" + name +".json";
		FILE* pFile = fopen(fileName.c_str(), "rb");
		
		if (pFile != NULL) {
			char buffer[65536];
			rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
			rapidjson::Document document;
			document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);

			// File was opened successfully and parsed into JSON,
			// now we create the instance

			list<Ogre::String> meshNames;
			if (document.HasMember("meshNames")) {
				for (int i = 0; i < document["meshNames"].Size(); i++) {
					meshNames.push_back(document["meshNames"][i].GetString());
				}
			} else {
				meshNames.push_back("ERROR.MESH.mesh");
			}


			// Parse data for the construction of the rigid body

			double restitution;
			if (document.HasMember("restitution")) {
				restitution = document["restitution"].GetDouble();
			} else {
				restitution = 0.0;
			}

			int massTemp;
			if (document.HasMember("mass")) {
				massTemp = document["mass"].GetInt();
			} else {
				massTemp = 1;
			}


			// Parse scale info
			Ogre::Vector3 scale = Ogre::Vector3(1, 1, 1);
			if (document.HasMember("scale")) {
				scale = parseVector3(document["scale"]);
			} else 
			{
				scale = Ogre::Vector3(1, 1, 1);
			}

			// Needed for collisions 
			// interaction legend by diana 
			// -1 = no interaction 
			// 1 = teapots meaning they disappear (for now) 
			// 2 = tuna can (ending... for now) 
			int interaction;
			if (document.HasMember("interaction")) {
				interaction = document["interaction"].GetInt();
			} else {
				interaction = -1; // this means there's no interaction 
			}


			string collisionShape;
			if (document.HasMember("collisionShape")) {
				collisionShape = document["collisionShape"].GetString();
			} else {
				collisionShape = "btBoxShape";
			}


			
			// temp vars used for parsing collision shape size data
			btVector3 colDim3 = btVector3(1,1,1);
			btScalar colScala = 1;
			btScalar colScalb = 1;

			/* Parse the CollisionShape and its size size */
			

			// if no collision shape size is specified in the json file
			// base its size off of the dimensions of the mesh
			// For simplicity, this only works if the dynamic object has
			// one mesh. 

			Ogre::Vector3 meshDimensions;
			std::string s = std::to_string(meshNames.size()); 
			

			if (!document.HasMember("collisionShapeSize")) {

				// XXX: The collision shape auto sizing functionality is experimental
				// and needs to be tested.

				Ogre::Entity* tempEntity = mSceneManager->createEntity(meshNames.front());
				colDim3 = ogreToBulletVector3(tempEntity->getMesh()->getBounds().getHalfSize());
				colScala = tempEntity->getMesh()->getBoundingSphereRadius(); // radius
				colScalb = tempEntity->getMesh()->getBounds().getSize()[1]; // height
			
				// apply scale
				colDim3 = btVector3(colDim3[0] * scale.x, colDim3[1] * scale.y, colDim3[2] * scale.z);
				colScala = colScala * scale.x;
				colScalb = colScalb * scale.y;

			} else if(document.HasMember("collisionShapeSize")) {
				/// Note: FIgure out why it keeps going into this if block instead of the first one 
				if (document["collisionShapeSize"].Size() == 3) {
					colDim3 = ogreToBulletVector3(parseVector3(document["collisionShapeSize"]));
				}
				colScala = document["collisionShapeSize"][0].GetDouble();	
				colScalb = document["collisionShapeSize"][1].GetDouble();   
			} else {
				OutputDebugString("ERROR! Need to specify the collisionshape size!");
				// default collision shape sizes
				colDim3 = btVector3(1,1,1);
				colScala = 1;
				colScalb = 1;
			}


			
			// holds the actual collision shape of the object
			btCollisionShape *colShape;
			
			if (collisionShape.compare("btSphereShape") == 0) {
				colShape = new btSphereShape(colScala);
			} else if(collisionShape.compare("btBoxShape") == 0) {
				colShape = new btBoxShape(colDim3);
			} else if(collisionShape.compare("btCylinderShape") == 0) {
				colShape = new btCylinderShape(colDim3);
			} else if(collisionShape.compare("btCapsuleShape") == 0) {
				colShape = new btCapsuleShape(colScala, colScalb);
			} else {
				// default to box shape if no valid collision shape was found
				colShape = new btBoxShape(btVector3(1,1,1));
			}
			
			// XXX: Implement these other shapes as needed!
			/*
			else if(collisionShape.compare("btConeShape") == 0) {
				
			} else if(collisionShape.compare("btMultiSphereShape") == 0) {
				
			} else if(collisionShape.compare("btConvexHullShape") == 0) {
				
			} else if(collisionShape.compare("btConvexTriangleMeshShape") == 0) {
				
			} else if(collisionShape.compare("btCompoundShape") == 0) {
				
			}
			*/
			

			// create the rigid body

			// set position to 0,0,0 later change when placing in the game
			btDefaultMotionState* fallMotionState =
				new btDefaultMotionState( btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
			btScalar mass = (btScalar) massTemp;
			btVector3 fallInertia(0, 0, 0);
			colShape->calculateLocalInertia(mass, fallInertia);
				

			btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, colShape, fallInertia);
			fallRigidBodyCI.m_restitution = restitution;

			// Create the rigid body
			btRigidBody* tempRigidBody = new btRigidBody(fallRigidBodyCI);

			// New way
			DynamicObject *newD = new DynamicObject(meshNames, colShape, Ogre::Vector3(0,0,0), interaction, scale);

			// put it into the library
			dynamicObjects.emplace(name, newD);

			std::fclose(pFile);

			return newD->clone(this->mSceneManager);
		} else {
			// no file was found
			return NULL;
		}
	}
}

StaticScenery * GameLibrary::getStaticScenery(string name, Ogre::Vector3 position, Ogre::Quaternion orientation) {

	unordered_map<string, StaticScenery*> ::iterator it = staticScenery.find(name);

	StaticScenery *statScenery;
	if (it != staticScenery.end())
	{
		//element found;
		statScenery = it->second;

		// create a clone of it.
		return statScenery->clone(this->mSceneManager, position, orientation, statScenery->mRigidBody->getUserIndex());
	} else {
		// element was not found.
		// load it in and create instance 

		std::string fileName = "../TeamProject/GameData/StaticScenery/" + name +".json";
		FILE* pFile = fopen(fileName.c_str(), "rb");

		if (pFile != NULL) {
			char buffer[65536];
			rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
			rapidjson::Document document;
			document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);

			// File was opened successfully and parsed into JSON,
			// now we create the instance

			string meshName;
			if (document.HasMember("meshName")) {
				meshName = document["meshName"].GetString();
			} else {
				meshName = "ERROR.MESH.mesh";
			}

			int interaction = -1; 

			// interaction legend 
			// -1 = no interaction 
			// 0 = player respawns  
			if (document.HasMember("interaction")) {
				OutputDebugString("interaction is set to 0 hurr\n"); 
				interaction = document["interaction"].GetInt();
			}




			// TODO: unique identifier system
			Ogre::Entity *tempEntity = mSceneManager->createEntity(meshName);
			tempEntity->setCastShadows(true);




			// Finished parsing the filess
			// Create the instance and put in the GameLibrary
			StaticScenery *newStaticScenery = new StaticScenery(tempEntity, position, orientation, interaction);



			// put it into the library
			staticScenery.emplace(name, newStaticScenery);

			std::fclose(pFile);

			return newStaticScenery->clone(this->mSceneManager, position, orientation, interaction);

		} else {
			// no file was found
			return NULL;
		}
	}
}






/*
	A stage file pairs prefabs with data that defines a relation to the prefab and the world.
	For now I can only think of two types of data like that.
	Orientation and position.


	Great so how do we pair it with that?

	We could have a stage load the scenenodes and stuff

	but I don't like that.

	I want to get a stage in memory.

	And do with it as I please.

	Solutions?

	We could make our own data strucutre for pairing location, orientation,.. etc with
	the prefab

	That sucks though because scenenode already basicly does this... however we have the problem
	that as soon as we attach the scenenode it will be rendered!

	Best solution

	Just add a position and orientation members to the prefab classes.
	Its weird because they wont be set till get stage but whatever not a big deal.
*/




Stage * GameLibrary::getStage(string name) {
	// element was not found.
	// load it in and create instance 
	
	std::string fileName = "../TeamProject/GameData/Stages/" + name +".json";
	FILE* pFile = fopen(fileName.c_str(), "rb");
		
	if(pFile != NULL) {
		char buffer[65536];
		rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
		rapidjson::Document document;
		document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
		
		// File was opened successfully and parsed into JSON,
		// now we create the instance, for anything not specified in the json file

		std::list<DynamicObject*> tempDynObjs;
		std::list<StaticScenery*> tempStaticScenery;
		std::list<Ogre::Light*> tempLights;
				
		// load dynamic objects
		if (document.HasMember("DynamicObjects")) {
			for (int i = 0; i < document["DynamicObjects"].Size(); i++) {
				
				string name = document["DynamicObjects"][i]["name"].GetString();
				
				Ogre::Vector3 position = Ogre::Vector3(0, 0, 0);
				Ogre::Quaternion rotation = Ogre::Quaternion(1, 0, 0, 0);

				if (document["DynamicObjects"][i].HasMember("position")) {
					double x = document["DynamicObjects"][i]["position"][0].GetDouble();
					double y = document["DynamicObjects"][i]["position"][1].GetDouble();
					double z = document["DynamicObjects"][i]["position"][2].GetDouble();
					position = Ogre::Vector3(x, y, z);
				} else {
					position = Ogre::Vector3(0, 0, 0);
				}

				if (document["DynamicObjects"][i].HasMember("Orientation")) {
					double w = document["DynamicObjects"][i]["Orientation"][0].GetDouble();
					double x = document["DynamicObjects"][i]["Orientation"][1].GetDouble();
					double y = document["DynamicObjects"][i]["Orientation"][2].GetDouble();
					double z = document["DynamicObjects"][i]["Orientation"][3].GetDouble();
					rotation = Ogre::Quaternion(w, x, y, z);
				} else {
					rotation = Ogre::Quaternion(1, 0, 0, 0);
				}


				DynamicObject* tempDynObj =  this->getDynamicObject(name);
				tempDynObj->setPosition(position);
				tempDynObj->setOrientation(rotation);
				tempDynObjs.push_back(tempDynObj);

				// psuedocode for collision filtering once this is set up
				// checks if it's an interactive object and if it is, add to phys manager 
				// list to handle specific collision checking 
				// if (tempDynObj->fallRigidBody->getUserIndex() != -1) 
				//    physmanager->collisionlist.push(tempDynObj)
			}
		}		

		// load StaticScenery
		if (document.HasMember("StaticScenery")) {
			for (int i = 0; i < document["StaticScenery"].Size(); i++) {
				
				
				string name = document["StaticScenery"][i]["name"].GetString();
				
				
				Ogre::Vector3 position = Ogre::Vector3(0, 0, 0);
				Ogre::Quaternion rotation = Ogre::Quaternion(1, 0, 0, 0);
//				int interaction = -1; 

				// interaction legend 
				// -1 = no interaction 
				// 0 = player respawns  
				//if (document.HasMember("interaction")) {
				//	OutputDebugString("interaction is set to 0 \n"); 
				//	interaction = document["interaction"].GetInt();
				//}

				if (document["StaticScenery"][i].HasMember("position")) {
					double x = document["StaticScenery"][i]["position"][0].GetDouble();
					double y = document["StaticScenery"][i]["position"][1].GetDouble();
					double z = document["StaticScenery"][i]["position"][2].GetDouble();
					position = Ogre::Vector3(x, y, z);
				} else {
					position = Ogre::Vector3(0, 0, 0);
				}

				
				if (document["StaticScenery"][i].HasMember("orientation")) {
					double w = document["StaticScenery"][i]["rotation"][0].GetDouble();
					double x = document["StaticScenery"][i]["rotation"][1].GetDouble();
					double y = document["StaticScenery"][i]["rotation"][2].GetDouble();
					double z = document["StaticScenery"][i]["rotation"][3].GetDouble();
					rotation = Ogre::Quaternion(w, x, y, z);
				} else {
					rotation = Ogre::Quaternion(1, 0, 0, 0);
				}

				StaticScenery* newStaticScenery = this->getStaticScenery(name, position, rotation);
				
				tempStaticScenery.push_back(newStaticScenery);
		
			} 
		}

		/*
		TODO: CODE THE LIGHTS!!!!
		// load lights
		if (document.HasMember("Lights")) {
			for (int i = 0; i < document["Lights"].Size(); i++) {
				string name = document["Lights"][i]["name"].GetString();
				// tempStaticScenery.push_back( lights or some shit );
			} 
		}

		*/



		// at this point the stage has been fully loaded.
		// create it and return it.


		/* the world will have a method called setUp stage
		   which will free any scene nodes from the previous stage
		   add everything in the stage data structure into the scene.
		*/

		Stage* stage = new Stage();

		stage->dynObjects = tempDynObjs;
		stage->staticScenery = tempStaticScenery;
		stage->lights = tempLights;
		
		// does it make sense to keep the stage data in a hashmap?
		// I dont think it does, since a stage contains references to prefabs, 
		// takes up uneeded space, we don't need to load stages very often.
		// and reading it from file is easy
		// stages.emplace(name, stage);

		return stage;
	} else {
		// the file was not found!
		// TODO: HANDLE GRACEFULLY!
		return NULL;
	}
}



btTriangleMesh* GameLibrary::ogreToBulletMesh(Ogre::MeshPtr mesh) {
	btTriangleMesh* btMesh = new btTriangleMesh();
	Ogre::Mesh::SubMeshIterator j = mesh->getSubMeshIterator();
    while (j.hasMoreElements()) {
        Ogre::SubMesh* submesh = j.getNext(); 
        
        int idxStart = submesh->indexData->indexStart;
        int nIdx = submesh->indexData->indexCount;
        
        Ogre::HardwareIndexBuffer* idxBuffer 
            = submesh->indexData->indexBuffer.get();
            
        Ogre::HardwareVertexBufferSharedPtr virtBuffer;
        Ogre::VertexDeclaration* virtDecl;
        
        if (submesh->useSharedVertices) {
            virtDecl = mesh->sharedVertexData->vertexDeclaration;
            assert(mesh->sharedVertexData->vertexBufferBinding->getBufferCount() > 0);
            virtBuffer = mesh->sharedVertexData->vertexBufferBinding->getBuffer(0);
        } else {
            virtDecl = submesh->vertexData->vertexDeclaration;
            assert(submesh->vertexData->vertexBufferBinding->getBufferCount() > 0);
            virtBuffer = submesh->vertexData->vertexBufferBinding->getBuffer(0);
        }
        
        unsigned char* pVert = static_cast<unsigned char*>(virtBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

        // need to lock the buffer since vertex data resides on GPU
        // and we need synchronization
        unsigned short* sindices = NULL;
        unsigned long* lindices = NULL;
        
        if (idxBuffer->getType() == Ogre::HardwareIndexBuffer::IT_16BIT) {
            sindices = static_cast<unsigned short*>(idxBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        } else if (idxBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT) {
            lindices = static_cast<unsigned long*>(idxBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        } else {
            assert(true == false);
        }
        
        const Ogre::VertexElement* elm = virtDecl->findElementBySemantic(Ogre::VES_POSITION, 0);
        int offset = elm->getOffset();
        // assert(elm->getType() == VET_FLOAT3);
        
        for (int k = idxStart; k < idxStart + nIdx; k += 3) {
            unsigned int indices[3];
            btVector3 vertices[3];
            
            if (idxBuffer->getType() == Ogre::HardwareIndexBuffer::IT_16BIT) {
                for (int l = 0; l < 3; ++l) {
                    indices[l] = sindices[k + l];
                }
            } else {
                for (int l = 0; l < 3; ++l) {
                    indices[l] = lindices[k + l];
                }
            }
            
            for (int l = 0; l < 3; ++l) { // for each vertex
                Ogre::Real* posVert = (Ogre::Real*)(pVert + indices[l] * virtBuffer->getVertexSize() + offset);
                for (int m = 0; m < 3; ++m) { // for each vertex component
                    vertices[l][m] = posVert[m];
                }
            }
            btMesh->addTriangle(vertices[0], vertices[1], vertices[2]);
        }
            
        idxBuffer->unlock();
        virtBuffer->unlock();
    }

	return btMesh;
}

Ogre::Vector3 GameLibrary::parseVector3(const rapidjson::Value& x) 
{
	return Ogre::Vector3(x[0].GetDouble(), x[1].GetDouble(), x[2].GetDouble());
}

btVector3 GameLibrary::ogreToBulletVector3(Ogre::Vector3 x) {
	return btVector3(x.x, x.y, x.z);
}