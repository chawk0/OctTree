#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Frustum.h"

class Camera
{
public:
	enum Mode { NONE = 0, FREE, TRACKING };

	Camera();
	~Camera();

	// what the fuck is this?
	float test(Vector3& vv)
	{
		float v[4], v2[4];
		v[0] = vv.x;
		v[1] = vv.y;
		v[2] = vv.z;
		v[3] = 1.0f;
		Matrix4 wvp, t;

		t.makeTranslate(-_position);
		wvp = _projectionMatrix * _viewMatrix * t;
		v2[0] = wvp.m[0] * v[0] + wvp.m[4] * v[1] + wvp.m[8] * v[2] + wvp.m[12] * v[3];
		v2[1] = wvp.m[1] * v[0] + wvp.m[5] * v[1] + wvp.m[9] * v[2] + wvp.m[13] * v[3];
		v2[2] = wvp.m[2] * v[0] + wvp.m[6] * v[1] + wvp.m[10] * v[2] + wvp.m[14] * v[3];
		v2[3] = wvp.m[3] * v[0] + wvp.m[7] * v[1] + wvp.m[11] * v[2] + wvp.m[15] * v[3];
		v2[0] /= v2[3];
		v2[1] /= v2[3];
		v2[2] /= v2[3];
		v2[3] = 1.0f;
		return v2[2];
	}


	void setProjection(Matrix4& projectionMatrix);
	void setProjection(int width, int height, float fovY, float zNear, float zFar);
	void setViewMatrix(Matrix4& m);
	void update();	
		
	void lookAt(const Vector3& target);
	void lookAt(const Vector3& target, const Vector3& up);
	void translateX(float d);
	void translateY(float d);
	void translateZ(float d);
	void rotateX(float t);
	void rotateY(float t);
	void rotateZ(float t);

	void setMode(int mode);
	void setTrackingTarget(const Vector3& v);
	void setTrackingUpVector(const Vector3& v);
	void setPosition(const Vector3& v);
	void setPosition(float x, float y, float z);

	const Matrix4& getViewMatrix() const;
	const Matrix4& getProjectionMatrix() const;

	int getMode() const;
	const Vector3& getPosition() const;
	float getX() const;
	float getY() const;
	float getZ() const;

	Vector3 getRight() const;
	Vector3 getUp() const;
	Vector3 getForward() const;

	Frustum& getFrustum();

private:
	void _calculateTrackOrientation();

	// don't allow these
	Camera(const Camera&);
	Camera& operator=(const Camera&);

	Vector3 _position, _target, _trackingUp;
	Vector3 _right, _up, _forward;
	Quaternion _quat;
	Matrix4 _viewMatrix, _projectionMatrix;
	Frustum _frustum;
	int _mode;
};
