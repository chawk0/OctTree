#include "AABox.h"

AABox::AABox():
	corner(0.0f, 0.0f, 0.0f),
	width(0.0f),
	height(0.0f),
	depth(0.0f)
{
}

AABox::AABox(const AABox& b):
	corner(b.corner),
	width(b.width),
	height(b.height),
	depth(b.depth)
{
}

const AABox& AABox::operator=(const AABox& b)
{
	corner = b.corner;
	width = b.width;
	height = b.height;
	depth = b.depth;

	return *this;
}

AABox::~AABox()
{
}

 int AABox::getType() const
{
	return CollisionVolume::AABOX;
}

void AABox::buildRenderBuffer()
{
	//
}

AABox& AABox::set(const Vector3& c, float w, float h, float d)
{
	corner = c;
			
	// if the corner of the box isn't the minimum vertex, then shift things
	// to make it so.
	if (w < 0)
	{
		w = -w;
		corner.x -= w;
	}

	if (h < 0)
	{
		h = -h;
		corner.y -= h;
	}

	if (d < 0)
	{
		d = -d;
		corner.z -= d;
	}

	width = w;
	height = h;
	depth = d;

	return *this;
}

Vector3 AABox::getPVertex(const Vector3& normal) const
{
	Vector3 p = corner;

	if (normal.x > 0)
		p.x += width;
	if (normal.y > 0)
		p.y += height;
	if (normal.z > 0)
		p.z += depth;

	return p;
}

Vector3 AABox::getNVertex(const Vector3& normal) const
{
	Vector3 n = corner;
			
	if (normal.x < 0)
		n.x += width;
	if (normal.y < 0)
		n.y += height;
	if (normal.z < 0)
		n.z += depth;

	return n;
}