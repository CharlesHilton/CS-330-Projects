#include <chrono>
#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <list>
#include <windows.h>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

using namespace std;

void processInput(GLFWwindow* window);

const float pi = glm::pi<float>();

std::chrono::time_point<std::chrono::steady_clock> lastKeyPressedTime[];

template<typename T>
bool in_range(T number, T min_value, T max_value) {
	return (number >= min_value && number <= max_value);
}

// Generate a random number of template type T.
template <typename T>
T Random(T min_val, T max_val) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<T> dis(min_val, max_val); // define the distribution
	return dis(gen);
}

glm::vec3 RandomColor(float lb = 0.1, float ub = 1.0) {
	float r = Random<float>(lb, ub);
	float g = Random<float>(lb, ub);
	float b = Random<float>(lb, ub);
	return glm::vec3(r, g, b);
}


glm::vec2 RandomDirectionVector() {
	float angle = glm::radians(Random<float>(0.0, 360.0));
	return glm::normalize(glm::vec2(cos(angle), sin(angle)));
}

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float width;
	float top, bottom, left, right;
	glm::vec2 location;
	glm::vec3 color;
	BRICKTYPE brick_type;
	ONOFF drawState;

	Brick(BRICKTYPE bt, glm::vec2 location, float width, glm::vec3 color) {
		this->brick_type = bt;
		this->location = location;
		this->width = width;
		this->color = color;

		this->drawState = ON;

		float halfside = width / 2;
		this->top = location.y + halfside;
		this->bottom = location.y - halfside;
		this->right = location.x + halfside;
		this->left = location.x - halfside;
	}

	void Draw()
	{
		if (drawState == ON)
		{
			float halfside = width / 2;
			top = location.y + halfside;
			bottom = location.y - halfside;
			right = location.x + halfside;
			left = location.x - halfside;

			glColor3d(color.r, color.g, color.b);
			glBegin(GL_POLYGON);
			glVertex2d(right, top);
			glVertex2d(right, bottom);
			glVertex2d(left, bottom);
			glVertex2d(left, top);

			glEnd();
		}
	}
};

class Circle
{
public:
	//float red, green, blue;
	glm::vec3 color;
	float radius;
	float speed = 0.01f;
	glm::vec2 location;
	glm::vec2 directionVector;
	float restitution = 0.9f;	// percent speed modification after collision with another object.

	Circle(glm::vec2 loc, glm::vec2 drcVec, float rad, glm::vec3 colr) {
		location = loc;
		directionVector = drcVec;
		radius = rad;
		color = colr;
	}

	float Area() { 
		return pi * pow(radius, 2);
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			/*if (glm::distance(this->location, brk->location) < (this->radius + brk->width/2))*/
			if (in_range(this->location.x, brk->left - radius, brk->right + radius) &&
				in_range(this->location.y, brk->bottom - radius, brk->top + radius))
			{
				this->directionVector = glm::normalize(this->location - brk->location);
				//speed *= restitution;
				this->speed *= 0.9f;
				//location += speed * directionVector;
				this->radius *= 0.9f;
				color = RandomColor();
				brk->color = RandomColor();
				if (brk->width < 0.3f) {
					brk->width *= 1.005f;
				}
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if (glm::distance(this->location, brk->location) < (this->radius + brk->width))
			{
				brk->drawState = OFF;
			}
		}
	}

	bool CheckCollision(Circle* otherCircle) {
		if (glm::distance(this->location, otherCircle->location) < (this->radius + otherCircle->radius)) {
			this->directionVector = glm::normalize(this->location - otherCircle->location);
			this->radius *= 0.99f;
			this->color = RandomColor();
			//this->speed *= Random<float>(0.5f, 1.5f);
			//otherCircle->directionVector = -this->directionVector;
			return true;
		}
		return false;
	}


	void Move() {
		// Flip direction vector for appropriate component when reaching the edge of the frame.
		if (this->location.x < -1.0) {
			directionVector.x = abs(directionVector.x);
		}
		
		if (this->location.x > 1.0) {
			directionVector.x = -abs(directionVector.x);
		}

		if (this->location.y < -1.0) {
			directionVector.y = abs(directionVector.y);
		}

		if (this->location.y > 1.0) {
			directionVector.y = -abs(directionVector.y);
		}

		location += (speed * directionVector); // Update location.
	}

	void Draw()
	{
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_POLYGON);
		for (float i = 0.0f; i < 360.0f; i++) {
			float angle = glm::radians(i);
		
			glVertex2f((cos(angle) * radius) + location.x, (sin(angle) * radius) + location.y);
		}
		glEnd();
	}
};

list<Circle> circles;
list<Brick> bricks;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(1080, 1080, "Random World of Circles", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);


	bricks.push_back(Brick(REFLECTIVE, glm::vec2(0.5, -0.33), 0.2, RandomColor()));
	bricks.push_back(Brick(REFLECTIVE, glm::vec2(0.5, -0.33), 0.2, RandomColor()));
	bricks.push_back(Brick(DESTRUCTABLE, glm::vec2(-0.5, 0.33), 0.2, RandomColor()));
	bricks.push_back(Brick(DESTRUCTABLE, glm::vec2(-0.5, -0.33), 0.2, RandomColor()));
	bricks.push_back(Brick(REFLECTIVE, glm::vec2(0, 0), 0.2, RandomColor()));

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
	
		processInput(window);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		//Movement
		for (auto itC = circles.begin(); itC != circles.end(); ) {
			// Check for collisions with bricks.
			for (auto itB = bricks.begin(); itB != bricks.end(); itB++) {
				itC->CheckCollision(&*itB);
			}
			// Check for collision with other circles (oc).
			bool collisionOccured = false;
			for (auto itOc = circles.begin(); itOc != circles.end(); ) {
				if (itC != itOc) {
					if (itC->CheckCollision(&*itOc)) {
						collisionOccured = true;
					}
				}
				itOc++;
			}
			itC->Move();
			itC->Draw();

			if (collisionOccured && itC->radius < 0.01) {
					itC = circles.erase(itC);
			}
			else
			{
				itC++;
			}
		}

		for (auto itB = bricks.begin(); itB != bricks.end(); ) {
			if (itB->drawState == OFF) {		// Remove destroyed bricks.
				itB = bricks.erase(itB);
			}
			else {								// Draw the brick.
				itB->Draw();
				itB++;
			}
		}

		// Draw bricks.
		//for (size_t b = 0; b < bricks.size(); b++) {
		//	bricks[b].Draw();
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}



void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
	/*	auto now = std::chrono::steady_clock::now();
		std::chrono::milliseconds deltaTime(100);
		if ((now - lastKeyPressedTime[GLFW_KEY_SPACE]) > deltaTime) {*/
			glm::vec2 baseLoc = glm::vec2(Random<float>(-1.0, 1.0), Random<float>(-1.0, 1.0));
			float baseRad = 0.05f;
			Circle cir(baseLoc, RandomDirectionVector(), baseRad, RandomColor());
			circles.push_back(cir);
		//	lastKeyPressedTime[GLFW_KEY_SPACE] = now;
		//}
	}
}