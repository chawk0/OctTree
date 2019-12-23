#include "Vector.h"

ostream& operator<<(ostream& os, const Vector2& v)
{
	os << "<" << v.x << ", " << v.y << ">";

	return os;
}

ostream& operator<<(ostream& os, const Vector3& v)
{
	os << "<" << v.x << ", " << v.y << ", " << v.z << ">";

	return os;
}

/*
inline Vector2 operator*(float s, const Vector2& v)
{
	return Vector2(v.x * s, v.y * s);
}

inline Vector3 operator*(float s, const Vector3& v)
{
	return Vector3(v.x * s, v.y * s, v.z * s);
}
*/