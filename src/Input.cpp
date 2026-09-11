#include "Input.h"


bool keysPressed[1024] = { false };
bool keysPressedLastFrame[1024] = { false };
glm::vec2 mouseDelta(0.0f, 0.0f);
glm::vec2 lastMousePosition(WIDTH / 2.0f, HEIGHT / 2.0f);
double scrollDelta = 1.0f;
bool firstMouseMovement = true;
bool isWindowFocused = true;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key > -1 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keysPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keysPressed[key] = false;
		}
	}

	if (keysPressed[GLFW_KEY_ESCAPE])
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isWindowFocused = false;
		firstMouseMovement = true;
	}
}

void MouseCallback(GLFWwindow* window, double rawPositionX, double rawPositionY)
{
	if (!isWindowFocused)
	{
		return;
	}

	float positionX = static_cast<float>(rawPositionX);
	float positionY = static_cast<float>(rawPositionY);

	if (firstMouseMovement)
	{
		lastMousePosition = glm::vec2(positionX, positionY);
		firstMouseMovement = false;
	}

	mouseDelta.x += positionX - lastMousePosition.x;
	mouseDelta.y += lastMousePosition.y - positionY;

	lastMousePosition = glm::vec2(positionX, positionY);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		isWindowFocused = true;
	}
}

void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	scrollDelta = yOffset;
}

void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}