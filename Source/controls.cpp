// Include GLFW
#include <GLFW/glfw3.h>
#include <iostream>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include <cmath>

// Window
GLFWwindow* mainWindow;

void setControlsWindow(GLFWwindow* window)
{
	mainWindow = window;
}

// Matrices
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}
glm::mat4 getViewMatrix() {
	return ViewMatrix;
}

// Initial position
glm::vec3 position = glm::vec3(100.0f, 0.0f, 100.0f);

glm::vec3 getCameraPosition() {
	return position;
}

void computeMatricesFromInputs(GLuint keycode, GLfloat deltaTime) {
	float speed = 80 * deltaTime;

	// Rotate right around X axis, 'w' = 119
	if (glfwGetKey(mainWindow, GLFW_KEY_W) == GLFW_PRESS && keycode == 119) {
		float angleY = glm::atan(position.z, position.y); // Get angle in radians
		angleY = angleY * 180 / glm::pi<float>();		// Rads to Degrees
		if (position.x < 0)								// Increase/Decrease the 
			angleY += speed;							// angle's degrees accordingly;
		else
			angleY -= speed;
		angleY = angleY * glm::pi<float>() / 180;		// back to Radians

		// Standard circle equation: r^2 = x^2 + y^2
		float radius = sqrtf(pow(position.y, 2) + pow(position.z, 2));
		position.y = radius * glm::cos(angleY);
		position.z = radius * glm::sin(angleY);
	}
	// Rotate left around X axis, 'x' = 120
	if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS && keycode == 120) {
		float angleY = glm::atan(position.z, position.y); // Get angle in radians
		angleY = angleY * 180 / glm::pi<float>();		// Rads to Degrees
		if (position.x < 0)								// Increase/Decrease the 
			angleY -= speed;							// angle's degrees accordingly;
		else
			angleY += speed;
		angleY = angleY * glm::pi<float>() / 180;		// back to Radians

		// Standard circle equation: r^2 = x^2 + y^2
		float radius = sqrtf(pow(position.y, 2) + pow(position.z, 2));
		position.y = radius * glm::cos(angleY);
		position.z = radius * glm::sin(angleY);
	}
	// Rotate right around Y axis, 'd' = 100
	if (glfwGetKey(mainWindow, GLFW_KEY_D) == GLFW_PRESS && keycode == 100) {
		float angleX = glm::atan(position.z, position.x); // Get angle in radians
		angleX = angleX * 180 / glm::pi<float>();		// Rads to Degrees
		angleX -= speed;								// Decrease the angle's degrees
		angleX = angleX * glm::pi<float>() / 180;		// back to Radians

		// Standard circle equation: r^2 = x^2 + y^2
		float radius = sqrtf(pow(position.x, 2) + pow(position.z, 2));
		position.x = radius * glm::cos(angleX);
		position.z = radius * glm::sin(angleX);
	}
	// Rotate left around Y axis, 'a' = 97
	if (glfwGetKey(mainWindow, GLFW_KEY_A) == GLFW_PRESS && keycode == 97) {
		float angleX = glm::atan(position.z, position.x); // Get angle in radians
		angleX = angleX * 180 / glm::pi<float>();		// Rads to Degrees
		angleX += speed;								// Increase the angle's degrees
		angleX = angleX * glm::pi<float>() / 180;		// back to Radians

		// Standard circle equation: r^2 = x^2 + y^2
		float radius = sqrtf(pow(position.x, 2) + pow(position.z, 2));
		position.x = radius * glm::cos(angleX);
		position.z = radius * glm::sin(angleX);
	}
	// Increase Z, '+' = 43
	if (((glfwGetKey(mainWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS) ||
		(glfwGetKey(mainWindow, GLFW_KEY_EQUAL) == GLFW_PRESS)) &&
		keycode == 43) {
		glm::vec3 diff = glm::vec3(0, 0, 0) - position;
		float length = sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
		glm::vec3 normalized = glm::vec3(diff.x / length, diff.y / length, diff.z / length);
		position += normalized * speed;

	}
	// Decrease Z, '-' = 45
	if (((glfwGetKey(mainWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) ||
		(glfwGetKey(mainWindow, GLFW_KEY_MINUS) == GLFW_PRESS)) &&
		keycode == 45) {
		glm::vec3 diff = glm::vec3(0, 0, 0) - position;
		float length = sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
		glm::vec3 normalized = glm::vec3(diff.x / length, diff.y / length, diff.z / length);
		position -= normalized * speed;
	}
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,					// Camera is here
		glm::vec3(0, 0, 0),			// and looks here : at the center of the galaxy
		glm::vec3(0, 1, 0)			// Head is on +z
	);
}