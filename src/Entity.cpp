#include "Entity.h"

 Vector3 Entity::getPosition() const
{
	return _position;
}

 Vector3 Entity::getVelocity() const
{
	return _velocity;
}

 Vector3 Entity::getAcceleration() const
{
	return _acceleration;
}

 float Entity::getMass() const
{
	return _mass;
}

 void Entity::setPosition(const Vector3& p)
{
	_position = p;
}

 void Entity::setPosition(float x, float y, float z)
{
	_position.x = x;
	_position.y = y;
	_position.z = z;
}

 void Entity::setVelocity(const Vector3& v)
{
	_velocity = v;
}

 void Entity::setAcceleration(const Vector3& a)
{
	_acceleration = a;
}

 void Entity::setMass(float m)
{
	_mass = m;
}

void Entity::destroy()
{
	_position = Vector3(0.0f, 0.0f, 0.0f);
	_velocity = Vector3(0.0f, 0.0f, 0.0f);
	_acceleration = Vector3(0.0f, 0.0f, 0.0f);
	_mass = 0.0f;

	if (_collisionVolume != NULL)
	{
		delete _collisionVolume;
		_collisionVolume = NULL;
	}

	if (_localBox != NULL)
	{
		delete _localBox;
		_localBox = NULL;
	}

	if (_mesh != NULL)
	{
		delete _mesh;
		_mesh = NULL;
	}
}