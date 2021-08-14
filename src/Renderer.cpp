#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cmath>
#include <string>
#include "Renderer.h"
#include "Constants.h"

// Clear openGL errors, makes it easier for debugging
void glClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

// Called on openGL error
bool glLogCall(const char* function, const char* file, int line) 
{
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << ")" << function << " " <<
			" " << file << " " << line << std::endl;
		return false;
	}
	return true;
}

// Clears openGL buffer bits
void Renderer::clear() const
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

// Renderer singleton instance
Renderer Renderer::s_Instance;

Renderer::Renderer() 
{
};

// Getting singleton instance
Renderer& Renderer::getInstance()
{
	return s_Instance;
}

// Setting the renderIndex to render the appropriate model on key press
void Renderer::setRenderIndex(unsigned int index)
{
	renderIndex = index;
}

void Renderer::setIsFindingDepth(bool findDepth) {
	this->isFindingDepth = findDepth;
}

// Renderer for rendering the axes
void Renderer::drawAxes(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection)
{
	va.bind();
	shader.bind();
	glLineWidth(4);

	glm::mat4 model = glm::mat4(1.0f) * glm::scale(glm::mat4(1.0f), glm::vec3(5));

	shader.setUniform4Mat("view", view);
	shader.setUniform4Mat("projection", projection);
	shader.setUniform4Mat("model", model);

	shader.setUniform4Vec("ourColor", glm::vec4(1.0, 0.0, 0.0, 1.0));
	GLCall(glDrawArrays(GL_LINES, 0, 2));

	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setUniform4Vec("ourColor", glm::vec4(0.0, 1.0, 0.0, 1.0));
	shader.setUniform4Mat("model", model);
	GLCall(glDrawArrays(GL_LINES, 0, 2));

	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.setUniform4Mat("model", model);
	shader.setUniform4Vec("ourColor", glm::vec4(0.0, 0.0, 1.0, 1.0));
	GLCall(glDrawArrays(GL_LINES, 0, 2));

	va.unbind();
	shader.unbind();
	glLineWidth(1);
}

// Renderer for drawing floor (with texture)
void Renderer::drawFloor(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture)
{
	// Binding vertex array and shader
	va.bind();
	if (!isFindingDepth) {
		shader.bind();
	}
	texture.bind();

	// Setting all uniform variables
	if (!isFindingDepth) {
		shader.setUniform4Mat("view", view);
		shader.setUniform4Mat("projection", projection);
		shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform3Vec("lightPosition", lightPos);
		shader.setUniform3Vec("viewPos", cameraPos);
		shader.setUniform3Vec("ourColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform1i("shininess", 32);
		shader.setUniform1i("textureStatus", 1);
	}
	

	// scale and align with XZ plane
	glm::mat4 model = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(100.0f));

	shader.setUniform4Mat("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// unbind for easier debugging
	va.unbind();
	if (!isFindingDepth) {
		shader.unbind();
	}
	texture.unbind();
}

void Renderer::drawCube(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection) {
	// Bind the vertex array and shader
	va.bind();
	shader.bind();



	shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setUniform4Mat("projection", projection);
	shader.setUniform4Mat("view", view);
	shader.setUniform3Vec("ourColor", glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 model = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(19.9)) * glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 2.0f));
	shader.setUniform4Mat("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glm::mat4 model1 = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(20)) * glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.5f, -2.0f));
	shader.setUniform4Mat("model", model1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glm::mat4 model2 = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(20)) * glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.5f, 2.0f));
	shader.setUniform4Mat("model", model2);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glm::mat4 model3 = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(20)) * glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, -2.0f));
	shader.setUniform4Mat("model", model3);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	// unbind for easier debugging
	va.unbind();
	shader.unbind();
}

void Renderer::drawModel(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection) {
	// Bind the vertex array and shader
	va.bind();
	shader.bind();



	shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setUniform4Mat("projection", projection);
	shader.setUniform4Mat("view", view);
	shader.setUniform3Vec("ourColor", glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 model = glm::mat4(1.0f)
		* glm::scale(glm::mat4(1.0f), glm::vec3(19.9)) * glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 2.0f));
	shader.setUniform4Mat("model", model);


	// unbind for easier debugging
	va.unbind();
	shader.unbind();
}


// Draw the model that is currently in use
void Renderer::drawObject(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, glm::mat4 modelRotMat, vector<glm::mat4> modelTransMat, float scaleFactor, glm::vec3 displacement)
{
	// Bind the vertex array and shader
	va.bind();
	if (!isFindingDepth) {
		shader.bind();
	}
	texture.bind();

	if (!isFindingDepth) {
		shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform3Vec("lightPosition", lightPos);
		shader.setUniform3Vec("viewPos", cameraPos);
		shader.setUniform4Mat("projection", projection);
		shader.setUniform4Mat("view", view);

		// handle texture display toggle
		if (isTextureEnabled)
		{
			shader.setUniform1i("textureStatus", 1);
			shader.setUniform3Vec("ourColor", glm::vec3(0.0f, 1.0f, 1.0f));
			shader.setUniform1i("shininess", 16);
		}
		else
		{
			shader.setUniform1i("textureStatus", 0);
			shader.setUniform3Vec("ourColor", glm::vec3(0.0f, 1.0f, 1.0f));
			shader.setUniform1i("shininess", 32);
		}
	}
	
	// draw cubes for model
	int numCubePieces = modelCubePositions.at(renderIndex).size();
	for (int i = 0; i < numCubePieces; i++)
	{
		// adjust position based on user input
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(displacement.x, 0.0f, displacement.z));
		glm::mat4 initialPos = glm::translate(glm::mat4(1.0f), modelPosition.at(renderIndex));

		// unit matrix * scaling input * xz_translation * model_translation (align with hole) * model_scale * model_rotation * model_cube_translation
		glm::mat4 model = glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor))
			* trans * initialPos
			* modelScale.at(renderIndex)
			* glm::translate(glm::mat4(1.0f), centerOfMass)
			* modelRotMat
			* glm::translate(glm::mat4(1.0f), -centerOfMass)
			* modelTransMat.at(i);

		shader.setUniform4Mat("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	// unbind for easier debugging
	va.unbind();
	if (!isFindingDepth) {
		shader.unbind();
	}
	
}

// Draw the lighting object
void Renderer::drawLightingSource(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos) {
	
	// Binding vertex array and shader
	va.bind();
	shader.bind();

	// Set uniform variables
	shader.setUniform4Mat("view", view);
	shader.setUniform4Mat("projection", projection);

	// Adjust the light source position & scale it
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(2.0f));
	shader.setUniform4Mat("model", model);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	va.unbind();
	shader.unbind();
}

// Draw the wall that is currently in use
void Renderer::drawWall(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, glm::mat4 modelRotMat, float scaleFactor, glm::vec3 displacement)
{
	// bind the vertex array and shader
	va.bind();
	if (!isFindingDepth) {
		shader.bind();
	}
	texture.bind();

	if (!isFindingDepth) {
		// set all uniform variables
		shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform3Vec("lightPosition", lightPos);
		shader.setUniform3Vec("viewPos", cameraPos);
		shader.setUniform4Mat("projection", projection);
		shader.setUniform4Mat("view", view);
		shader.setUniform1i("shininess", 32);

		// handle texture display toggle
		if (isTextureEnabled)
		{
			shader.setUniform1i("textureStatus", 1);
			shader.setUniform3Vec("ourColor", glm::vec3(1.0f, 1.0f, 1.0f));
		}
		else
		{
			shader.setUniform1i("textureStatus", 0);
			shader.setUniform3Vec("ourColor", glm::vec3(0.63f, 0.63f, 0.63f));
		}
	}

	int numWallPieces = wallCubePositions.at(renderIndex).size();
	for (int i = 0; i < numWallPieces; i++)
	{
		// calculate model matrix for each object and pass it to shader before drawing
		glm::mat4 initialPos = glm::translate(glm::mat4(1.0f), modelPosition.at(renderIndex));

		// unit matrix * wall_scale * x_translation * wall_translation (align with XZ plane) * wall_cube_scale * wall_cube_translate
		glm::mat4 model = glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor))
			* initialPos
			* rotationMatrix
			* modelScale.at(renderIndex)
			* glm::translate(glm::mat4(1.0f), wallCubePositions.at(renderIndex).at(i));

		shader.setUniform4Mat("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
  
	// unbind for easier debugging
	va.unbind();
	if (!isFindingDepth) {
		shader.unbind();
	}
	texture.unbind();
}

void Renderer::updateCenterOfMass()
{
	int numCubePieces = modelCubePositions.at(renderIndex).size();
	centerOfMass = glm::vec3(0.0f);
	glm::vec3 numOfCubes = glm::vec3(0.0f);

	for (int i = 0; i < numCubePieces; i++)
	{
		if (modelCubePositions.at(renderIndex).at(i).x != 0)
		{
			centerOfMass.x += modelCubePositions.at(renderIndex).at(i).x;
			numOfCubes.x++;
		}

		if (modelCubePositions.at(renderIndex).at(i).y != 0)
		{
			centerOfMass.y += modelCubePositions.at(renderIndex).at(i).y;
			numOfCubes.y++;
		}

		if (modelCubePositions.at(renderIndex).at(i).z != 0)
		{
			centerOfMass.z += modelCubePositions.at(renderIndex).at(i).z;
			numOfCubes.z++;
		}
	}
	centerOfMass.x /= numOfCubes.x;
	centerOfMass.y /= numOfCubes.y;
	centerOfMass.z /= numOfCubes.z;
}

