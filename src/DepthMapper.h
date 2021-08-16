#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "Shader.h"

static float near = 1.0f;
static float far = 125.0f; // set to render all the map for the lighting + shadows


class DepthMapper {
private:
	unsigned int depthMapFBO;
	unsigned int depthCubeMap;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

public:
	DepthMapper();

	void Draw(Shader* depthShader, glm::vec3 lightPos, std::function<void()> doDraw);

	void bind();


};

