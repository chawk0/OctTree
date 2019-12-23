/*
	represents a geometric plane as a normal vector and a scalar D.
	given Ax + By + Cz + D = 0, the first 3 terms represent n dot p,
	so n dot p + D = 0, thus n dot p = -D, and 
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"

class Plane
{
	public:
		Plane() { }

		Plane(const Plane& p):
			n(p.n), d(p.d)
		{
			//
		}

		Plane(float A, float B, float C, float D):
			n(A, B, C), d(D)
		{
			float mag = n.length();
			n /= mag;
			D /= mag;
		}

		Plane(const Vector3& point, const Vector3& normal):
			n(normal), d(-normal.dot(point))
		{
			//
		}
		
		~Plane() { }

		float distanceTo(const Vector3& p)
		{
			return n.dot(p) + d;
		}

		void setCoefficients(float A, float B, float C, float D)
		{
			n.x = A; n.y = B; n.z = C;
			float mag = n.length();
			n /= mag;
			d = D / mag;
		}

		Vector3 n;
		float d;
};