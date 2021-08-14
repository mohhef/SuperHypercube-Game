#pragma once
#include <glm/glm.hpp>
#include <vector>

enum Movement {
	x, y, z
};

class RotationMat {
public:
	glm::vec3 currentRot;
	glm::vec3 maxRot;

	glm::mat4 rotMatrix;

	std::vector<Movement> movementQueue; // required for proper movemement

	glm::mat4 getMatrix();
	void setMatrix(glm::mat4 mat);
	RotationMat(glm::vec3 rot);
	void setHard(glm::vec3 hardRotation);
	void setSoft(glm::vec3 softRotation);
	glm::mat4 convertCurrentToHard();
	void updateRotation(float time);
	void reset();
};


