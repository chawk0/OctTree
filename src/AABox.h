/*
	represents an axis-aligned bounding box as 1 corner point and 3 side lengths for each dimension.
	the getPVertex and getNVertex functions return a "positive" and "negative" vertex, respectively,
	which represent directions that are parallel and antiparallel to the provided normal.

	more on this technique at http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
*/

#pragma once
#include "ChawkFortress.h"

#include "Vector.h"
#include "CollisionVolume.h"

class AABox : public CollisionVolume
{
public:
	Vector3 corner;
	float width, height, depth;

	AABox();
	AABox(const AABox& b);
	const AABox& operator=(const AABox& b);

	~AABox();

	int getType() const;
	void buildRenderBuffer();

	AABox& set(const Vector3& c, float w, float h, float d);

	Vector3 getPVertex(const Vector3& normal) const;
	Vector3 getNVertex(const Vector3& normal) const;
};
