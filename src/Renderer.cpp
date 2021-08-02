#include <iostream>
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

Renderer::Renderer() {};

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

void Renderer::setRenderCombinedRot(bool rot) {
	combinedRot = rot;
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

// Renderer for drawing the mesh
void Renderer::drawMesh(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, float scaleFactor)
{
	// Binding vertex array and shader
	va.bind();
	if (!isFindingDepth) {
		shader.bind();

		//Setting all uniform variables
		shader.setUniform4Mat("view", view);
		shader.setUniform4Mat("projection", projection);
		shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform3Vec("lightPosition", lightPos);
		shader.setUniform3Vec("viewPos", cameraPos);
		shader.setUniform3Vec("ourColor", glm::vec3(0.5f, 0.5f, 0.5f));
		shader.setUniform1i("textureStatus", 0);
		shader.setUniform1i("shininess", 32);
	}
	

	// Materials to be used by the following for loop
	glm::mat4 model;
	glm::mat4 model2;

	// for loop that draws vertically and horizontally all the lines from -50 to +50 to cover the entire mesh in a 100x100 grid
	for (float current = -50.0f; current <= 50.0f; current += 1.0f) {

		// center line * scaling
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 0.0f, current))
			* glm::scale(glm::mat4(1.0f), glm::vec3(100));
		shader.setUniform4Mat("model", model);
		GLCall(glDrawArrays(GL_LINES, 0, 2));

		// rotate line * center line * scaling
		model2 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 0.0f, current))
			* glm::scale(glm::mat4(1.0f), glm::vec3(100));
		shader.setUniform4Mat("model", model2);
		GLCall(glDrawArrays(GL_LINES, 0, 2));
	}

	// Unbinding for easier debugging
	va.unbind();
	if (!isFindingDepth) {
		shader.unbind();
	}
	
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

// Renderer for drawing the static models and walls at the corners of the map
void Renderer::drawStaticObjects(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& textureWall, Texture& textureModel) 
{
	// Translation to put non-centered models
	vector<glm::mat4> corners = 
	{
		glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, 0.0f, -30.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 0.0f, -30.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, 0.0f, 35.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 0.0f, 35.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 30.0f, -30.0f)),
	};

	// Binding vertex array and shader
	va.bind();
	if (!isFindingDepth) {
		shader.bind();
	}
	textureModel.bind();

	// Set uniform variables
	if (!isFindingDepth) {
		shader.setUniform3Vec("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setUniform3Vec("lightPosition", lightPos);
		shader.setUniform3Vec("viewPos", cameraPos);
		shader.setUniform4Mat("projection", projection);
		shader.setUniform4Mat("view", view);
	}
	

	// Handle texture display toggle
	if (!isFindingDepth) {
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
	
	// Draw objects
	for (int index = 0; index < modelCubePositions.size(); index++) 
	{
		if (index == s_Instance.renderIndex)
			continue;

		// Drawing all the cubes for the associated object at index i
		int numCubePieces = modelCubePositions.at(index).size();
		for (int i = 0; i < numCubePieces; i++) 
		{
			glm::mat4 initialPos = glm::translate(glm::mat4(1.0f), modelPosition.at(index));
			glm::mat4 modelCubePos = glm::translate(glm::mat4(1.0f), modelCubePositions.at(index).at(i));
			glm::mat4 modelCubeScale = modelScale.at(index);
			glm::mat4 translation = index > s_Instance.renderIndex ? corners.at(index - 1) : corners.at(index);
			glm::mat4 model = glm::mat4(1.0f)
				* translation
				* initialPos
				* modelCubeScale
				* modelCubePos;

			shader.setUniform4Mat("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	textureModel.unbind();
	textureWall.bind();

	if (!isFindingDepth) {
		// Set uniform variables
		shader.setUniform1i("shininess", 32);

		// Handle texture display toggle
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
	
	
	// Draw walls
	for (int index = 0; index < modelCubePositions.size(); index++) 
	{
		if (index == s_Instance.renderIndex)
			continue;

		// Drawing all the wall cubes for the associated object at index i
		int numWallPieces = wallCubePositions.at(index).size();
		for (int i = 0; i < numWallPieces; i++) 
		{
			glm::mat4 initialPos = glm::translate(glm::mat4(1.0f), modelPosition.at(index));
			glm::mat4 wallCubePos = glm::translate(glm::mat4(1.0f), wallCubePositions.at(index).at(i));
			glm::mat4 translation = index > s_Instance.renderIndex ? corners.at(index - 1) : corners.at(index);

			glm::mat4 model = glm::mat4(1.0f)
				* initialPos
				* translation
				* modelScale.at(renderIndex)
				* wallCubePos;

			shader.setUniform4Mat("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	va.unbind();
	if (!isFindingDepth) {
		shader.unbind();
	}
	textureWall.unbind();
}

void Renderer::drawBoundary(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, vector<glm::mat4> modelRotMat, vector<glm::mat4> modelTransMat, float scaleFactor, glm::vec3 displacement)
{
	va.bind();
	shader.bind();
	glLineWidth(4);

	shader.setUniform4Mat("projection", projection);
	shader.setUniform4Mat("view", view);
	GLfloat timeVal = glfwGetTime();
	GLfloat greenVal = (sin(timeVal) / 2) + 0.5;
	shader.setUniform4Vec("ourColor", glm::vec4(0.0, greenVal, 0.0, 1.0));

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
			* modelRotMat.at(i)
			* glm::translate(glm::mat4(1.0f), -centerOfMass)
			* modelTransMat.at(i);

		shader.setUniform4Mat("model", model);
		glDrawArrays(GL_LINES, 0, 24);
	}
	// unbind for easier debugging
	va.unbind();
	shader.unbind();
	glLineWidth(1);
}

// Draw the model that is currently in use
void Renderer::drawObject(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, vector<glm::mat4> modelRotMat, vector<glm::mat4> modelTransMat, float scaleFactor, glm::vec3 displacement)
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
			* modelRotMat.at(i)
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

//Draw the lighting object
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
void Renderer::drawWall(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, vector<glm::mat4> modelRotMat, float scaleFactor, glm::vec3 displacement)
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
	
	
	//using the same roation as the object will work 
	if (combinedRot == true) {
		rotationMatrix = modelRotMat.at(1);
	}

	int numWallPieces = wallCubePositions.at(renderIndex).size();
	for (int i = 0; i < numWallPieces; i++)
	{
		// calculate model matrix for each object and pass it to shader before drawing
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(displacement.x, 0.0f, 0.0f));
		glm::mat4 initialPos = glm::translate(glm::mat4(1.0f), modelPosition.at(renderIndex));

		// unit matrix * wall_scale * x_translation * wall_translation (align with XZ plane) * wall_cube_scale * wall_cube_translate
		glm::mat4 model = glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor))
			* trans
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