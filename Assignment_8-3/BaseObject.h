#pragma once
#include <glm/glm.hpp>
class  BaseObject
{
public:
	glm::vec2 location;
	glm::vec2 direction;
	glm::vec3 color;
	virtual void Draw() = 0;
	virtual void CheckCollision(BaseObject* otherBaseObject) = 0;
};

