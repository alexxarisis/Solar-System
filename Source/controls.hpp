#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void setControlsWindow(GLFWwindow* newWindow);
void computeMatricesFromInputs(GLuint keycode, GLfloat deltaTime);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getCameraPosition();

#endif