/*
	float-based Vector2 and Vector3 classes.
*/

#pragma once
#include "ChawkFortress.h"

//#include "Matrix.h"
//#include "Quaternion.h"
//#include "Camera.h"
//#include "Engine.h"
//#include "Log.h"
//#include "MatrixStack.h"
//#include "Perlin.h"
//#include "SceneObject.h"
//#include "Sketch.h"
//#include "Texture.h"
//#include "Timer.h"
//#include "World.h"

#ifndef PI
#define PI	3.1415926535897932384626433832795f
#endif

#ifndef PI_OVER_180
#define PI_OVER_180  (PI / 180.0f)
#endif

#ifndef EPSILON
#define EPSILON	0.000001f
#endif

//------------------------------------------------------------------------------

class Vector2
{
	public:
		float x, y;

		Vector2() { }
		Vector2(float newX, float newY) { x = newX; y = newY; }
		Vector2(float* v) { x = v[0]; y = v[1]; }
		Vector2(const Vector2& v) { x = v.x; y = v.y; }

		// Array access
		float& operator[](int idx) const
		{
			return ((float*)this)[idx];
		}
	
		// Assignment
		Vector2& operator=(const Vector2& v)
		{
			x = v.x;
			y = v.y;
	
			return *this;
		}
	
		// Equality
		bool operator==(const Vector2& v) const
		{
			return ((x == v.x) && (y == v.y));
		}
	
		// Inequality
		bool operator!=(const Vector2& v) const
		{
			return ((x != v.x) || (y != v.y));
		}
	
		// Addition
		Vector2 operator+(const Vector2& v) const
		{
			return Vector2(x + v.x, y + v.y);
		}
	
		// Increment
		Vector2& operator+=(const Vector2& v)
		{
			x += v.x;
			y += v.y;
	
			return *this;
		}
	
		// Subtraction
		Vector2 operator-(const Vector2& v) const
		{
			return Vector2(x - v.x, y - v.y);
		}
	
		// Negation
		Vector2 operator-() const
		{
			return Vector2(-x, -y);
		}
	
		// Decrement
		Vector2& operator-=(const Vector2& v)
		{
			x -= v.x;
			y -= v.y;
	
			return *this;
		}
	
		// Scalar multiply and assignment
		Vector2& operator*=(float s)
		{
			x *= s;
			y *= s;
	
			return *this;
		}
	
		// Scalar division and assignment
		Vector2& operator/=(float s)
		{
			float r = 1.0f / s;
	
			x *= r;
			y *= r;
	
			return *this;
		}
	
		// Scalar multiply
		Vector2 operator*(float s) const
		{
			return Vector2(x * s, y * s);
		}
	
		// Scalar divide
		Vector2 operator/(float s) const
		{
			float r = 1.0f / s;
	
			return Vector2(x * r, y * r);
		}
	
		// Dot product
		float dot(const Vector2& v) const
		{
			return x * v.x + y * v.y;
		}
	
		// Length
		float length() const
		{
			return sqrtf(x * x + y * y);
		}
	
		// Length squared
		float lengthSqr() const
		{
			return (x * x + y * y);
		}
	
		// Normalize
		Vector2& normalize()
		{
			float r = 1.0f / length();
			x *= r;
			y *= r;

			return *this;
		}
};

//------------------------------------------------------------------------------

class Vector3
{
	public:
		float x, y, z;
	
		Vector3() { }
		Vector3(float newX, float newY, float newZ) { x = newX; y = newY; z = newZ; }
		Vector3(float* v) { x = v[0]; y = v[1]; z = v[2]; }
		Vector3(const Vector3& v) { x = v.x; y = v.y; z = v.z; }

		// Array access
		float& operator[](int idx)
		{
			return ((float*)this)[idx];
		}
	
		// Assignment
		Vector3& operator=(const Vector3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
	
			return *this;
		}
	
		// Equality
		bool operator==(const Vector3& v) const
		{
			return ((x == v.x) && (y == v.y) && (z == v.z));
		}
	
		// Inequality
		bool operator!=(const Vector3& v) const
		{
			return ((x != v.x) || (y != v.y) || (z != v.z));
		}

		bool isEqualEpsilon(const Vector3& v) const
		{
			return (fabs(x - v.x) < EPSILON) && (fabs(y - v.y) < EPSILON) && (fabs(z - v.z) < EPSILON);
		}

		bool isNotEqualEpsilon(const Vector3& v) const
		{
			return !isEqualEpsilon(v);
		}
	
		// Addition
		Vector3 operator+(const Vector3& v) const
		{
			return Vector3(x + v.x, y + v.y, z + v.z);
		}
	
		// Increment
		Vector3& operator+=(const Vector3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
	
			return *this;
		}
	
		// Subtraction
		Vector3 operator-(const Vector3& v) const
		{
			return Vector3(x - v.x, y - v.y, z - v.z);
		}
	
		// Negation
		Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}
	
		// Decrement
		Vector3& operator-=(const Vector3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
	
			return *this;
		}
	
		// Scalar multiply and assignment
		Vector3& operator*=(float s)
		{
			x *= s;
			y *= s;
			z *= s;
	
			return *this;
		}
	
		// Scalar division and assignment
		Vector3& operator/=(float s)
		{
			float r = 1.0f / s;
	
			x *= r;
			y *= r;
			z *= r;
	
			return *this;
		}
	
		// Scalar multiply
		Vector3 operator*(float s) const
		{
			return Vector3(x * s, y * s, z * s);
		}
	
		// Scalar divide
		Vector3 operator/(float s) const
		{
			float r = 1.0f / s;
	
			return Vector3(x * r, y * r, z * r);
		}
	
		// Dot product
		float dot(const Vector3& v) const
		{
			return x * v.x + y * v.y + z * v.z;
		}
	
		// Dot product vs. float[3]
		float dot(const float* v) const
		{
			return x * v[0] + y * v[1] + z * v[2];
		}
	
		// Cross product
		Vector3 cross(const Vector3& v) const
		{
			return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
		}
	
		// Length
		float length() const
		{
			return sqrtf(x * x + y * y + z * z);
		}
	
		// Length squared
		float lengthSqr() const
		{
			return (x * x + y * y + z * z);
		}
	
		// Normalize
		Vector3& normalize()
		{
			float r = 1.0f / length();
			x *= r;
			y *= r;
			z *= r;

			return *this;
		}
};

ostream& operator<<(ostream& os, const Vector2& v);
ostream& operator<<(ostream& os, const Vector3& v);
// Scalar multiply as float*Vector2
//Vector2 operator*(float s, const Vector2& v);
// Scalar multiply as float*Vector3
//Vector3 operator*(float s, const Vector3& v);

inline Vector2 operator*(float s, const Vector2& v)
{
	return Vector2(v.x * s, v.y * s);
}

inline Vector3 operator*(float s, const Vector3& v)
{
	return Vector3(v.x * s, v.y * s, v.z * s);
}

