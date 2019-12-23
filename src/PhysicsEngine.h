#pragma once
#include "ChawkFortress.h"

#include "Entity.h"
#include "Octree.h"
#include "Shader.h"
#include "Camera.h"


class PhysicsEngine
{
public:
	PhysicsEngine()
	{
		//
	}

	~PhysicsEngine()
	{
		cleanup();
	}

	//void addEntity(?
	void init(Octree* octree);
	void update(double dt);
	void renderEntities(Shader* shader, Camera* camera);
	void cleanup();

//private:
	Array<Entity*> _entities;
};
