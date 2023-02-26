#pragma once
#include "BaseObject.h"
class Polygon : public BaseObject
{
public:
	unsigned int nSides;
	float size;
	// Inherited via BaseObject
	virtual void Draw() override;
	virtual void CheckCollision(BaseObject* otherBaseObject) override;
	virtual void setNSides(unsigned int n) {
		nSides = n;
	}
};

