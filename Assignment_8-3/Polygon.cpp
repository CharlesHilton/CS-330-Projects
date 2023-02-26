#include "Polygon.h"
#include <GL/glew.h>

void Polygon::Draw()
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_POLYGON);
	for (unsigned int i = 0; i < nSides; i++) {
		float angle = glm::radians((float)i * ((float)nSides / 360));
		glVertex2f((cos(angle) * size) + location.x, (sin(angle) * size) + location.y);
	}
	glEnd();
}

void Polygon::CheckCollision(BaseObject* otherBaseObject)
{
	float distance = glm::distance(this->location, otherBaseObject->location);
}
