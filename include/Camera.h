#pragma once

#include <glm/glm.hpp>


struct Camera
{
	glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3 (1.0f, 0.0f, 0.0f);
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };

	float yaw = -90.0f;
	float pitch = 0.0f;
	float fov = 90.0f;

	const float movementSpeed = 0.05f;
	const float mouseSensitivity = 0.1f;
	const float scrollSensitivity = 1.0f;

	bool isOrbitCamera = true;
	glm::vec3 orbitTarget = glm::vec3(0.0f);
	float orbitDistance = 5.0f;
	bool cPressedLastFrame = false;


	Camera();

	void Camera::processInputs(bool* keys, bool* keysLastFrame, glm::vec2& mouseDelta, double scrollDelta);
	void updateVectors();

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix(float aspectRatio) const;
};