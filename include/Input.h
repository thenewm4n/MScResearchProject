#pragma once

#include "Camera.h"
#include "Constants.h"
#include <GLFW/glfw3.h>


extern bool keysPressed[1024];
extern bool keysPressedLastFrame[1024];
extern glm::vec2 mouseDelta;
extern glm::vec2 lastMousePosition;
extern double scrollDelta;
extern bool firstMouseMovement;
extern bool isWindowFocused;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseCallback(GLFWwindow* window, double rawPositionX, double rawPositionY);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void ResizeCallback(GLFWwindow* window, int width, int height);