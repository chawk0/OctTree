#include "Matrix.h"

Quaternion Matrix4::toQuaternion()
{
	Quaternion result;
	float w4;

	result.w = sqrtf(1.0f + m[0] + m[5] + m[10]) / 2.0f;
	w4 = result.w * 4.0f;
	result.x = (m[6] - m[9]) / w4;
	result.y = (m[8] - m[2]) / w4;
	result.z = (m[1] - m[4]) / w4;

	return result;
}

ostream& operator<<(ostream& os, const Matrix4& m)
{
	cout.precision(6);
    cout.flags(cout.flags() | ios::showpoint | ios::fixed);
    cout << endl;
    for (int i = 0; i < 4; i++)
        cout << "[ " << m.m[0 + i] << ", " << m.m[4 + i] << ", " << m.m[8 + i] << ", " << m.m[12 + i] << "]" << endl;
    cout << endl;

	return os;
}