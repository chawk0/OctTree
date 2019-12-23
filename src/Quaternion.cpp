#include "Quaternion.h"

Matrix4 Quaternion::toMatrix()
{
	Matrix4 result;
	float x2, y2, z2, xy, xz, yz, wx, wy, wz;

	result.makeZero();
	result.m[15] = 1.0f;

	x2 = x * x; y2 = y * y; z2 = z * z;

	xy	= x * y;
	xz	= x * z;
	yz	= y * z;
	wx	= w * x;
	wy	= w * y;
	wz	= w * z;
	
	result.m[0] = 1.0f - 2.0f * (y2 + z2);
	result.m[1] = 2.0f * (xy + wz);
	result.m[2] = 2.0f * (xz - wy);

	result.m[4] = 2.0f * (xy - wz);
	result.m[5] = 1.0f - 2.0f * (x2 + z2);
	result.m[6] = 2.0f * (yz + wx);

	result.m[8] = 2.0f * (xz + wy);
	result.m[9] = 2.0f * (yz - wx);
	result.m[10] = 1.0f - 2.0f * (x2 + y2);

	return result;
}

Quaternion Quaternion::operator*(const Quaternion& q)
{
	Quaternion result;

	result.x = q.w*x + q.x*w + q.y*z - q.z*y;
	result.y = q.w*y + q.y*w + q.z*x - q.x*z;
	result.z = q.w*z + q.z*w + q.x*y - q.y*x;
	result.w = q.w*w - q.x*x - q.y*y - q.z*z;

	result.normalize();
	return result;
}

Quaternion& Quaternion::operator*=(const Quaternion& q)
{
	float rx, ry, rz, rw;

	rx = q.w*x + q.x*w + q.y*z - q.z*y;
	ry = q.w*y + q.y*w + q.z*x - q.x*z;
	rz = q.w*z + q.z*w + q.x*y - q.y*x;
	rw = q.w*w - q.x*x - q.y*y - q.z*z;

	x = rx; y = ry; z = rz; w = rw;
	normalize();

	return *this;
}

