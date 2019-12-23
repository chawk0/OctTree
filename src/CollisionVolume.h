#pragma once
#include "ChawkFortress.h"

class CollisionVolume
{
public:
	enum Type { NONE = 0, AABOX, OBOX, SPHERE, ELLIPSOID, CYLINDER };

	virtual int getType() const = 0;
	virtual void buildRenderBuffer() = 0;

private:
	RenderData _renderData;
};
