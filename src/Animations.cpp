#include "Animations.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

glm::mat4 RotationMat::getMatrix() {
	return rotMatrix;
}

void RotationMat::setMatrix(glm::mat4 mat) {
	rotMatrix = mat;
}

RotationMat::RotationMat(glm::vec3 rot) : currentRot(rot), maxRot(rot) {
	rotMatrix = convertCurrentToHard();
}

// resets the model to its original position
void RotationMat::reset() {
	currentRot = glm::vec3(0.0f);
	maxRot = glm::vec3(0.0f);
	this->rotMatrix = convertCurrentToHard();
}

// converts the current rotation vertex to a matrix
glm::mat4 RotationMat::convertCurrentToHard() {
	glm::mat4 model = glm::mat4(1.0f)
		* glm::rotate(glm::mat4(1.0f), glm::radians(currentRot.x), glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(currentRot.y), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(currentRot.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return model;
}

// set the rotation matrix to directly reflect desired rotation (no movement)
void RotationMat::setHard(glm::vec3 hardRotation) {
	currentRot = hardRotation;
	maxRot = hardRotation;
	rotMatrix = convertCurrentToHard();
}

// set up the rotation to start moving towards rotation
void RotationMat::setSoft(glm::vec3 softRotation) {
	maxRot.x += softRotation.x;
	maxRot.y += softRotation.y;
	maxRot.z += softRotation.z;

	// creating a movement queue so that rotations don't occur along multiple axes at the same time
	// as that would not produce the desired end-goal rotation
	if (softRotation.x != 0.0f) {
		movementQueue.push_back(Movement::x);
	}
	if (softRotation.y != 0.0f) {
		movementQueue.push_back(Movement::y);
	}
	if (softRotation.z != 0.0f) {
		movementQueue.push_back(Movement::z);
	}
}
void RotationMat::updateRotation(float currentTime) {
	static float prevTime = currentTime;

	float speed = (currentTime - prevTime) * 1500.0f;
	prevTime = currentTime;

	float tolerance = 1.0f;

	// checking to see if this action is at the head of the movement queue
	if (movementQueue.size() > 0 && movementQueue[0] == Movement::x) {
		// checking to see if rotation required in +ve or -ve x axis
		if (currentRot.x < maxRot.x) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = speed;
			if (currentRot.x + speed > maxRot.x) {
				updateSpeed = maxRot.x - currentRot.x;
				currentRot.x = maxRot.x;
			}
			else {
				currentRot.x += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(1.0f, 0.0f, 0.0f)) * rotMatrix;
		}
		else if (currentRot.x > maxRot.x) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = -speed;
			if (currentRot.x - speed < maxRot.x) {
				updateSpeed = maxRot.x - currentRot.x;
				currentRot.x = maxRot.x;
			}
			else {
				currentRot.x += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(1.0f, 0.0f, 0.0f)) * rotMatrix;
		}

		// if done rotating, clear first element for the next movements to occur
		if (currentRot.x == maxRot.x) {
			movementQueue.erase(movementQueue.begin());
		}
	}
	
	// checking to see if this action is at the head of the movement queue
	if (movementQueue.size() > 0 && movementQueue[0] == Movement::y) {
		// checking to see if rotation required in +ve or -ve y axis
		if (currentRot.y < maxRot.y) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = speed;
			if (currentRot.y + speed > maxRot.y) {
				updateSpeed = maxRot.y - currentRot.y;
				currentRot.y = maxRot.y;
			}
			else {
				currentRot.y += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(0.0f, 1.0f, 0.0f)) * rotMatrix;
		}
		else if (currentRot.y > maxRot.y) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = -speed;
			if (currentRot.y - speed < maxRot.y) {
				updateSpeed = maxRot.y - currentRot.y;
				currentRot.y = maxRot.y;
			}
			else {
				currentRot.y += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(0.0f, 1.0f, 0.0f)) * rotMatrix;
		}

		// if done rotating, clear first element for the next movements to occur
		if (currentRot.y == maxRot.y) {
			movementQueue.erase(movementQueue.begin());
		}
	}

	// checking to see if this action is at the head of the movement queue
	if (movementQueue.size() > 0 && movementQueue[0] == Movement::z) {
		// checking to see if rotation required in +ve or -ve z axis
		if (currentRot.z < maxRot.z) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = speed;
			if (currentRot.z + speed > maxRot.z) {
				updateSpeed = maxRot.z - currentRot.z;
				currentRot.z = maxRot.z;
			}
			else {
				currentRot.z += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(0.0f, 0.0f, 1.0f)) * rotMatrix;
		}
		else if (currentRot.z > maxRot.z) {
			// speed to update rotation, to be adjusted if the speed brings it passed the desired rotation
			float updateSpeed = -speed;
			if (currentRot.z - speed < maxRot.z) {
				updateSpeed = maxRot.z - currentRot.z;
				currentRot.z = maxRot.z;
			}
			else {
				currentRot.z += updateSpeed;
			}

			// rotate based on rotation speed
			rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(updateSpeed), glm::vec3(0.0f, 0.0f, 1.0f)) * rotMatrix;
		}

		// if done rotating, clear first element for the next movements to occur
		if (currentRot.z == maxRot.z) {
			movementQueue.erase(movementQueue.begin());
		}
	}
}
