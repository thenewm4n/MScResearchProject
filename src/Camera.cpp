#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>


glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
	return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
}

Camera::Camera()
{
	updateVectors();
}

void Camera::processInputs(bool* keys, bool* keysLastFrame, glm::vec2& mouseDelta, double scrollDelta)
{
	// Toggle camera mode
	if (keys[GLFW_KEY_C] && !cPressedLastFrame)
	{
		isOrbitCamera = !isOrbitCamera;
	}
	
	cPressedLastFrame = keys[GLFW_KEY_C];

	// Mouse Input
	yaw += mouseSensitivity * mouseDelta.x;
	pitch += mouseSensitivity * mouseDelta.y;
	pitch = std::clamp(pitch, -89.0f, 89.0f);
	mouseDelta = glm::vec2(0.0f);				// Reset so movement doesn't carry to next frame
	
	updateVectors();

	// Keyboard Input
	if (isOrbitCamera)
	{	// Orbit cam
		position = orbitTarget - (forward * orbitDistance);
		orbitDistance -= scrollSensitivity * scrollDelta;
	}
	else
	{
		// Free cam
		velocity = glm::vec3(0.0f);

		if (keys[GLFW_KEY_W]) velocity += forward * movementSpeed;
		if (keys[GLFW_KEY_A]) velocity -= right * movementSpeed;
		if (keys[GLFW_KEY_S]) velocity -= forward * movementSpeed;
		if (keys[GLFW_KEY_D]) velocity += right * movementSpeed;
		if (keys[GLFW_KEY_SPACE]) velocity += up * movementSpeed;
		if (keys[GLFW_KEY_LEFT_SHIFT]) velocity -= up * movementSpeed;

		position += velocity;
	}
}

void Camera::updateVectors()
{
	// Calculate new forward, right and up vectors according to yaw and pitch
	glm::vec3 forwardDirection;
	forwardDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	forwardDirection.y = sin(glm::radians(pitch));
	forwardDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	
	forward = glm::normalize(forwardDirection);
	right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, forward));
}