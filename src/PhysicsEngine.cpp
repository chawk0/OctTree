#include "PhysicsEngine.h"

void PhysicsEngine::init(Octree* octree)
{
	//
}

void PhysicsEngine::update(double dt)
{
	//
}

void PhysicsEngine::renderEntities(Shader* shader, Camera* camera)
{
	//
}

void PhysicsEngine::cleanup()
{
	for (int i = 0; i < _entities.size(); ++i)
		if (_entities[i] != NULL)
			delete _entities[i];
	_entities.free();
}