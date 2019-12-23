#include "Camera.h"

Camera::Camera():
	_position(0.0f, 0.0f, 0.0f),
	_target(0.0f, 0.0f, 0.0f),
	_trackingUp(0.0f, 0.0f, 0.0f),
	_right(1.0f, 0.0f, 0.0f),
	_up(0.0f, 1.0f, 0.0f),
	_forward(0.0f, 0.0f, -1.0f),
	_mode(Mode::FREE)
{
	_quat.makeIdentity();
	_viewMatrix.makeIdentity();
	_projectionMatrix.makeIdentity();
	_frustum.setCameraPos(_position);
	_frustum.setCameraVectors(_right, _up, _forward);
}

Camera::~Camera()
{
}

void Camera::setProjection(Matrix4& projectionMatrix)
{
	_projectionMatrix = projectionMatrix;
}

void Camera::setMode(int mode)
{
	// if we're exiting tracking mode, copy the current orientation
	// to the quat so we don't "snap" to the old orientation
	if (_mode == Mode::TRACKING && mode != Mode::TRACKING)
		_quat = _viewMatrix.toQuaternion();
	_mode = mode;
}

void Camera::setTrackingTarget(const Vector3& v)
{
	if (_mode == Mode::TRACKING)
	{
		_target = v;
		update();
	}
}

void Camera::setTrackingUpVector(const Vector3& v)
{
	_trackingUp = v;
	_trackingUp.normalize();
}

void Camera::setPosition(const Vector3& v)
{
	_position = v;
	_frustum.setCameraPos(_position);
	if (_mode == Mode::TRACKING)
		update();
}

void Camera::setPosition(float x, float y, float z)
{
	_position.x = x;
	_position.y = y;
	_position.z = z;
	_frustum.setCameraPos(_position);
	if (_mode == Mode::TRACKING)
		update();
}

void Camera::setProjection(int width, int height, float fovY, float zNear, float zFar)
{
	_projectionMatrix.makeProjection((float)width / (float)height, fovY, zNear, zFar);
	_frustum.setProjectionProperties((float)width / (float)height, fovY, zNear, zFar);
}

void Camera::setViewMatrix(Matrix4& m)
{
	_viewMatrix = m;

	_right.x = _viewMatrix.m[0];
	_right.y = _viewMatrix.m[4];
	_right.z = _viewMatrix.m[8];

	_up.x = _viewMatrix.m[1];
	_up.y = _viewMatrix.m[5];
	_up.z = _viewMatrix.m[9];

	// gotta love that god damned Z-flip
	_forward.x = -_viewMatrix.m[2];
	_forward.y = -_viewMatrix.m[6];
	_forward.z = -_viewMatrix.m[10];
}

const Matrix4& Camera::getViewMatrix() const
{
	return _viewMatrix;
}

const Matrix4& Camera::getProjectionMatrix() const
{
	return _projectionMatrix;
}

int Camera::getMode() const
{
	return _mode;
}

const Vector3& Camera::getPosition() const
{
	return _position;
}

float Camera::getX() const
{
	return _position.x;
}

float Camera::getY() const
{
	return _position.y;
}

float Camera::getZ() const
{
	return _position.z;
}

Vector3 Camera::getRight() const
{
	return _right;
}

Vector3 Camera::getUp() const
{
	return _up;
}

Vector3 Camera::getForward() const
{
	return _forward;
}

Frustum& Camera::getFrustum()
{
	return _frustum;
}

void Camera::update()
{
	if (_mode == Mode::FREE)
	{
		_quat.normalize();
		_viewMatrix = _quat.toMatrix();

		_right.x = _viewMatrix.m[0];
		_right.y = _viewMatrix.m[4];
		_right.z = _viewMatrix.m[8];

		_up.x = _viewMatrix.m[1];
		_up.y = _viewMatrix.m[5];
		_up.z = _viewMatrix.m[9];

		// gotta love that god damned Z-flip
		_forward.x = -_viewMatrix.m[2];
		_forward.y = -_viewMatrix.m[6];
		_forward.z = -_viewMatrix.m[10];

		_frustum.setCameraVectors(_right, _up, _forward);
	}
	else if (_mode == Mode::TRACKING)
	{
		_calculateTrackOrientation();
	}
}

void Camera::_calculateTrackOrientation()
{
	//
}

void Camera::lookAt(const Vector3& target)
{
	lookAt(target, Vector3(0.0f, 1.0f, 0.0f));
}

void Camera::lookAt(const Vector3& target, const Vector3& up)
{
	Vector3 newRight, newUp, newForward;
	Vector3 projRight;

	newForward = target - _position;
	newForward.normalize();

	if (newForward.isNotEqualEpsilon(up))
	{
		newRight = newForward.cross(up);
		newUp = newRight.cross(newForward);
		newRight.normalize();
		newUp.normalize();
	}
	else
	{
		// hmm...
		projRight = Vector3(_up.x, 0.0f, 0.0f);
		newUp = projRight.cross(newForward);
		newRight = newForward.cross(newUp);
		newUp.normalize();
		newRight.normalize();
	}

	_right = newRight;
	_up = newUp;
	_forward = newForward;

	_viewMatrix.makeIdentity();
	_viewMatrix.setRight(_right);
	_viewMatrix.setUp(_up);
	_viewMatrix.setForward(-_forward);
}

void Camera::translateX(float d)
{
	_position += (_right * d);
	_frustum.setCameraPos(_position);
	if (_mode == Mode::TRACKING)
		update();
}

void Camera::translateY(float d)
{
	_position += (_up * d);
	_frustum.setCameraPos(_position);
	if (_mode == Mode::TRACKING)
		update();
}

void Camera::translateZ(float d)
{
	_position += (_forward * d);
	_frustum.setCameraPos(_position);
	if (_mode == Mode::TRACKING)
		update();
}

void Camera::rotateX(float t)
{
	Quaternion q;
	float ra;

	if (_mode != Mode::TRACKING)
	{
		q.makeIdentity();
		ra = PI_OVER_180 * t / 2.0f;
		q.x = sinf(ra);
		q.w = cosf(ra);

		_quat *= q;
		update();
	}
}

void Camera::rotateY(float t)
{
	Quaternion q;
	float ra;

	if (_mode != Mode::TRACKING)
	{
		q.makeIdentity();
		ra = PI_OVER_180 * t / 2.0f;
		q.y = sinf(ra);
		q.w = cosf(ra);

		_quat *= q;
		update();
	}
}

void Camera::rotateZ(float t)
{
	Quaternion q;
	float ra;

	if (_mode != Mode::TRACKING)
	{
		q.makeIdentity();
		ra = PI_OVER_180 * t / 2.0f;
		q.z = sinf(ra);
		q.w = cosf(ra);

		_quat *= q;
		update();
	}
}

