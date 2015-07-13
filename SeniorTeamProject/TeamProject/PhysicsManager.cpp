#include "PhysicsManager.h"
#include "BulletCollision\CollisionDispatch\btCollisionWorld.h"
#include "World.h"
#include "Player.h"
#include "HUD.h"

using namespace std;

/* Constructor */
PhysicsManager::PhysicsManager() {
	/* Setup the bullet Physics world. */
    _broadphase = new btDbvtBroadphase();
    _collisionConfiguration = new btDefaultCollisionConfiguration();
    _dispatcher = new btCollisionDispatcher(_collisionConfiguration);
    _solver = new btSequentialImpulseConstraintSolver();
    _world = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _collisionConfiguration);
    _world->setGravity(btVector3(0, -150, 0));
}

/* Deconstructor */
PhysicsManager::~PhysicsManager() {
    delete _world;
    delete _solver;
    delete _collisionConfiguration;
    delete _dispatcher;
    delete _broadphase;
}

/* Adds a plane */
void PhysicsManager::addPlane() {
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -20, 0)));

	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	groundRigidBodyCI.m_restitution = 0.5f;
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

	_world->addRigidBody(groundRigidBody);
}

/* Update function for the Bullet world */
void PhysicsManager::stepSimulation(float time, World* mWorld) {

	/* Creates the callback to detect whether objects are touching in the world */
	struct MyContactResultCallback : public btDiscreteDynamicsWorld::ContactResultCallback
	{
		bool m_connected;

		MyContactResultCallback() 
		{
			m_connected = false;
		}
				
		virtual btScalar   addSingleResult(btManifoldPoint& cp,	
			const btCollisionObjectWrapper* colObj0Wrap,int partId0,
			int index0,const btCollisionObjectWrapper* colObj1Wrap,
			int partId1,int index1)  
		{
			/* Sets boolean to true if there's been a contact */
			m_connected = true; 
			return 0;
		}
	};

	_world->stepSimulation(time, 10);
	
	IPhysObject *objToRm; 
	bool remove = false; 

	/* Update all physics objects */
	for (std::list<IPhysObject*>::iterator it = physObjects.begin(); it != physObjects.end(); it++) 
	{
		if (it._Ptr->_Myval->fallRigidBody->getUserIndex() != -1) 
		{
			MyContactResultCallback result;

			/* Does the testing between player and the objects we want to iterate through */
			_world->contactPairTest(mWorld->mPlayer->getDynamicObject()->fallRigidBody, it._Ptr->_Myval->fallRigidBody, result);

			/* Gets the result of the contact pair test and see whether objects have contacted each other */
			if (result.m_connected) {

				/* Increments score if this object with index 1 has been touched by the player */
				if (it._Ptr->_Myval->fallRigidBody->getUserIndex() == 1) 
				{
					remove = true;
					objToRm = it._Ptr->_Myval;

					mWorld->display->incrementScore();
					break; 
				}

				/* Ends the level if this object with index 2 has been touched by the player */
				else if (it._Ptr->_Myval->fallRigidBody->getUserIndex() == 2)
				{
					mWorld->display->displayEnding(true);
				}
			}
		}
		/* Synchronizes object with Bullet */
		it._Ptr->_Myval->synchWithBullet();
	}
	
	if (remove)
	{
		/* Removes object and rigid body from Bullet */
		_world->removeRigidBody(objToRm->fallRigidBody);
		physObjects.remove(objToRm);

		/* Removes from ogre world so it's no longer visible */
		mWorld->SceneManager()->destroyEntity(objToRm->ent->getName().c_str()); 
	}

	// Removes static scenery 

	StaticScenery *staticObjectToRm; 
	bool staticObjRemove = false; 

	// Checks if player is colliding with a static scenary with interaction # 0 
	for (std::list<StaticScenery*>::iterator it = mWorld->stage->staticScenery.begin(); it != mWorld->stage->staticScenery.end(); it++) {
		
		if (it._Ptr->_Myval->mRigidBody->getUserIndex() != -1)
		{
			MyContactResultCallback call;

			/* Does the testing between player and the objects we want to iterate through */
			_world->contactPairTest(mWorld->mPlayer->getDynamicObject()->fallRigidBody, it._Ptr->_Myval->mRigidBody, call);

			if (call.m_connected)
			{
				if (it._Ptr->_Myval->mRigidBody->getUserIndex() == 0)
				{
					// Respawn player 
					btTransform position; 
					mWorld->mPlayer->getDynamicObject()->fallRigidBody->getMotionState()->getWorldTransform(position);
					position.setOrigin(btVector3(-2645, 58, -918));
					mWorld->mPlayer->getDynamicObject()->fallRigidBody->setWorldTransform(position);
				}

				/* Increments score if this object with index 1 has been touched by the player */
				else if (it._Ptr->_Myval->mRigidBody->getUserIndex() == 1) 
				{
					staticObjRemove = true;
					staticObjectToRm = it._Ptr->_Myval;

					mWorld->display->incrementScore();
					break; 
				}

				else if (it._Ptr->_Myval->mRigidBody->getUserIndex() == 2)
				{
					mWorld->display->displayEnding(true);
				}

				else if (it._Ptr->_Myval->mRigidBody->getUserIndex() == 3)
				{
					mWorld->display->displayHint(true);
				}
			}
		}
	}


	if (staticObjRemove)
	{
		/* Removes object and rigid body from Bullet */
		_world->removeRigidBody(staticObjectToRm->mRigidBody);
		mWorld->stage->staticScenery.remove(staticObjectToRm);

		/* Removes from ogre world so it's no longer visible */
		mWorld->SceneManager()->destroyEntity(staticObjectToRm->mEntity->getName().c_str()); 
	}	
}