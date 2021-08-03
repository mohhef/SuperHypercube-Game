#include "DepthMapper.h"

#include <functional>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Constants.h"


DepthMapper::DepthMapper() {
	GLCall(glGenFramebuffers(1, &depthMapFBO));

	GLCall(glGenTextures(1, &depthCubeMap));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap));

	// setting up the textures for all 6 faces for point lighting
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO));
	GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0));

	// telling the frame buffer to not read or draw, as it isn't necessary
	GLCall(glDrawBuffer(GL_NONE));
	GLCall(glReadBuffer(GL_NONE));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}


void DepthMapper::Draw(Shader* depthShader, glm::vec3 lightPos, std::function<void()> doDraw)  
{
	float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

	// creating the shadow projection perspective
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect,
		near, far);

	// creating shadow transforms for the depth maps
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj*
		glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0)
			, glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj*
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0)
			, glm::vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProj*
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0)
			, glm::vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProj*
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0)
			, glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj*
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0)
			, glm::vec3(0.0, -1.0, 0.0)));

	// load up framebuffer to analyze the depths with
	GLCall(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO));
	GLCall(glClear(GL_DEPTH_BUFFER_BIT));

	Renderer::getInstance().setIsFindingDepth(true);
	// Set up shadow matrices for each face of the cube (to analyze the depth map from all 6 faces)
	depthShader->bind();
	for (int i = 0; i < 6; ++i) {
		depthShader->setUniform4Mat("shadowCubeFaces[" + std::to_string(i) + "]", shadowTransforms[i]);
	}

	depthShader->setUniform1f("map_range", far);
	depthShader->setUniform3Vec("lightPosition", lightPos);

	// Draw everything that needs to be within the depth map
	doDraw();
	Renderer::getInstance().setIsFindingDepth(false);

	// Clean up depth map from buffer
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	glViewport(0, 0, WIDTH, HEIGHT);
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}


void DepthMapper::bind() {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
}

