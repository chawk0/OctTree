/*
	this class represents a view frustum volume and contains methods to check
	for point, axis-aligned bounding box, and sphere intersection with the volume
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Plane.h"
#include "AABox.h"

class Frustum
{
	public:
		Frustum() { }
		~Frustum() { }

		void setCameraPos(const Vector3& pos);
		void setCameraVectors(const Vector3& right, const Vector3& up, const Vector3& forward);
		void setProjectionProperties(float aspectRatio, float fovY, float nearZ, float farZ);
		void extractPlanes(const Matrix4& wvp);	// extracts planes from the WVP matrix, rather than geometric computation

		bool containsPointR(const Vector3& p);	// Radar approach
		int containsAABB(const AABox& box);
		int containsSphere();

	private:
		Vector3 _pos, _right, _up, _forward;
		float _nearZ, _farZ;
		float _fovY, _fovYFactor, _fovXFactor, _aspectRatio;
		Plane _planes[6];
};
