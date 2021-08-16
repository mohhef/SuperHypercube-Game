#pragma once
#include <glm/glm.hpp>

enum class KEY {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class Camera
{
public:
	// Constants
	const float DEFAULT_ROTATION_SPEED = 30.0f;
	const float DEFAULT_SENSITIVITY = 0.1f;
	const float DEFAULT_ZOOM = 45.0f; // FOV

	glm::vec3 position; // Camera position
	glm::vec3 front; // Camera direction
	glm::vec3 up; // Up direction relative to camera
	glm::vec3 right; // Right direction relative to camera
	glm::vec3 worldUp; // Up direction relative to world
	glm::vec3 target; // Target position

	float cameraSpeed; // Factor for camera rotation around world axis
	float mouseSensitivity; // Factor for camera rotation around camera axis
	float zoom; // Total zoom factor
	void moveCamera(glm::vec3 displacement);
	void lookAt(glm::vec3 pos);
	void resetPos(glm::vec3 position);

	Camera(glm::vec3 position, glm::vec3 worldUp, glm::vec3 target);

	void processMovement(KEY key, float deltaTime); // Handle rotation (around world axis) inputs
	void tiltCamera(float yOffset); // Handle camera tilt (rotation around camera's X-axis)
	void panCamera(float xOffset); // Handle camera pan (rotation around camera's Y-axis)
	void zoomCamera(float yOffset); // Handle camera zoom

	glm::mat4 getViewMatrix(); // Return LookAt matrix

private:
	void rotateCameraGlobal(float factor, glm::vec3 axis); // Handles rotations for processMovement
};