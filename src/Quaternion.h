/*
	float-based Quaternion class.
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Shader.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

class Quaternion
{
	public:
		float x, y, z, w;

		Quaternion() { }
		Quaternion(float newX, float newY, float newZ, float newW)
		{
			x = newX; y = newY; z = newZ; w = newW;
		}
		Quaternion(const Quaternion& q)
		{
			x = q.x; y = q.y; z = q.z; w = q.w;
		}

		Quaternion& makeIdentity()
		{
			x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f;
			return *this;
		}

		Quaternion& makeFromAxisAngle(const Vector3& v, float a)
		{
			float ra, sa;

			ra = PI_OVER_180 * a / 2.0f;
			sa = sinf(ra);
			x = v.x * sa;
			y = v.y * sa;
			z = v.z * sa;
			w = cosf(ra);

			return *this;
		}
		
		Quaternion& makeFromAxisAngle(float ax, float ay, float az, float a)
		{
			float ra, sa;

			ra = PI_OVER_180 * a / 2.0f;
			sa = sinf(ra);
			x = ax * sa;
			y = ay * sa;
			z = az * sa;
			w = cosf(ra);

			return *this;
		}

		Quaternion& normalize()
		{
			float m = sqrtf(x * x + y * y + z * z + w * w);
			float i;
			if (m != 0.0f) { i = 1.0f / m; x *= i; y *= i; z *= i; w *= i; }
			return *this;
		}

		Matrix4 toMatrix();

		Quaternion& operator=(const Quaternion& q)
		{
			x = q.x; y = q.y; z = q.z; w = q.w;
			return *this;
		}

		Quaternion operator*(const Quaternion& q);
		Quaternion& operator*=(const Quaternion& q);
};


