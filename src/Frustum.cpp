#include "Frustum.h"

void Frustum::setCameraPos(const Vector3& pos)
{
	_pos = pos;
}

void Frustum::setCameraVectors(const Vector3& right, const Vector3& up, const Vector3& forward)
{
	_right = right;
	_up = up;
	_forward = forward;
}

void Frustum::setProjectionProperties(float aspectRatio, float fovY, float nearZ, float farZ)
{
	_aspectRatio = aspectRatio;
	_fovY = fovY;
	_fovYFactor = tanf(fovY * PI / 180.0f / 2.0f);
	_fovXFactor = _fovYFactor * _aspectRatio;
	_nearZ = nearZ;
	_farZ = farZ;
}

bool Frustum::containsPointR(const Vector3& p)
{
	// this method is implemented using the "Radar Approach" described at
	// http://www.lighthouse3d.com/tutorials/view-frustum-culling/radar-approach-testing-points/

	// progressively check z bounds, y bounds, then x bounds using simpler logic and fewer dot products
	// then a brute-force for loop over 6 planes.  combine this with the fact that it only needs the
	// camera position and basis vectors, instead of needing to extract plane information and recompute
	// plane normals and coefficients and all that.  pshaw!

	// start by projecting the view-space position vector of the point in question on the frustum's forward direction.
	// the resulting scalar is the z distance in view space.
	Vector3 viewSpaceP = p - _pos;
	float pz = _forward.dot(viewSpaceP);

	// check z bounds
	if (pz < _nearZ || pz > _farZ)
		return false;
	else
	{
		// check y bounds
		// compute the height of the frustum at z = pz.
		// h = pz * 2 * tan(fovY / 2)
		float py = _up.dot(viewSpaceP);
		float halfH = pz * _fovYFactor;
		
		if (py < -halfH || py > halfH)
			return false;
		else
		{
			// check x bounds
			// same computation as the y, but incorporate aspect ratio
			float px = _right.dot(viewSpaceP);
			float halfW = pz * _fovXFactor;

			if (px < -halfW || px > halfW)
				return false;
		}
	}

	// the victor!  inside the frustum!
	return true;
}

void Frustum::extractPlanes(const Matrix4& wvp)
{
	// this method extracts the frustum bounding planes from the proj-view-world matrix
	// as described in http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf.
	// nifty.

#define _m(r,c) wvp.m[r+c*4-5]

	// right plane (+X)
	_planes[0].setCoefficients(_m(4,1) - _m(1,1),
							   _m(4,2) - _m(1,2), 
							   _m(4,3) - _m(1,3),
							   _m(4,4) - _m(1,4));

	// left plane (-X)
	_planes[1].setCoefficients(_m(4,1) + _m(1,1),
							   _m(4,2) + _m(1,2), 
							   _m(4,3) + _m(1,3),
							   _m(4,4) + _m(1,4));

	// top plane (+Y)
	_planes[2].setCoefficients(_m(4,1) - _m(2,1),
							   _m(4,2) - _m(2,2), 
							   _m(4,3) - _m(2,3),
							   _m(4,4) - _m(2,4));

	// bottom plane (-Y)
	_planes[3].setCoefficients(_m(4,1) + _m(2,1),
							   _m(4,2) + _m(2,2), 
							   _m(4,3) + _m(2,3),
							   _m(4,4) + _m(2,4));

	// near plane (+Z)
	_planes[4].setCoefficients(_m(4,1) + _m(3,1),
							   _m(4,2) + _m(3,2), 
							   _m(4,3) + _m(3,3),
							   _m(4,4) + _m(3,4));

	// far plane (-Z)
	_planes[5].setCoefficients(_m(4,1) - _m(3,1),
							   _m(4,2) - _m(3,2), 
							   _m(4,3) - _m(3,3),
							   _m(4,4) - _m(3,4));
#undef _m
}

int Frustum::containsAABB(const AABox& box)
{
	// returns 0 if the box is fully outside of the frustum,
	// returns 1 if the box intersects the frustum,
	// returns 2 if the box is fully inside of the frustum.

	// start off by assuming the AABox is fully inside the frustum
	int result = 2;

	for (int p = 0; p < 6; ++p)
	{
		// if the "positive" vertex is behind the plane, you know the rest of the
		// box is too.  if the box is fully behind the plane, i.e. in the negative
		// halfspace, it's fully outside of the frustum, since all the planes have
		// inward-pointing normals.
		if (_planes[p].distanceTo(box.getPVertex(_planes[p].n)) < 0)
			return 0;
		// else if the "negative" vertex is behind the plane, you can at least
		// say there's an intersection with that plane.  it does NOT mean, however,
		// that the box intersects the frustum, which are finite areas inside of
		// the planes.  by continuing to loop through the other planes, if it's
		// found that the box's positive vertex is fully outside another plane, then
		// there's still an opportunity to return the correct result.
		else if (_planes[p].distanceTo(box.getNVertex(_planes[p].n)) < 0)
			result = 1;
	}

	// if we've made it this far, either the box tested fully inside of all planes
	// and the initial result of 2 (fully inside) wasn't changed, or one of the
	// planes tested the box to be intersecting.
	return result;
}

int Frustum::containsSphere()
{
	return 0;
}

