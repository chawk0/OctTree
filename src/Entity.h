#pragma once
#include "ChawkFortress.h"

#include "AABox.h"
#include "Vector.h"
#include "CollisionVolume.h"
#include "Mesh.h"
#include "Octree.h"

class Entity
{
	public:
		Entity():
			_position(0.0f, 0.0f, 0.0f),
			_velocity(0.0f, 0.0f, 0.0f),
			_acceleration(0.0f, 0.0f, 0.0f),
			_mass(0.0f),
			_collisionVolume(NULL),
			_localBox(NULL),
			_mesh(NULL)
		{
			//
		}

		~Entity()
		{
			destroy();
		}

		Vector3 getPosition() const;
		Vector3 getVelocity() const ;
		Vector3 getAcceleration() const;
		float getMass() const;

		void setPosition(const Vector3& p);
		void setPosition(float x, float y, float z);
		void setVelocity(const Vector3& v);
		void setAcceleration(const Vector3& a);
		void setMass(float m);
		
		void destroy();

	private:
		Vector3 _position;
		Vector3 _velocity;
		Vector3 _acceleration;
		float _mass;

		CollisionVolume* _collisionVolume;
		AABox* _localBox;
		Mesh* _mesh;
};
