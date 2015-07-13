#ifndef PhysicsManager_H
#define PhysicsManager_H
#include "IPhysObject.h"
#include <list>

class World;

class PhysicsManager
{
private:

public:
	PhysicsManager();
	~PhysicsManager();
	void addBall(btVector3* initialPos);
	void addPlane();
	void stepSimulation(float time, World* mWorld);

	btBroadphaseInterface* _broadphase;
    btDefaultCollisionConfiguration* _collisionConfiguration;
    btCollisionDispatcher* _dispatcher;
    btSequentialImpulseConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _world;

	/* Stores all physics objects in the world */
	std::list<IPhysObject*> physObjects;

};


#endif
