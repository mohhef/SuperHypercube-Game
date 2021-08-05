#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cmath>
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
	initializeCharacters();
	scale = 1.0f;
	x_score = 25.0f;
	y_score = 25.0f;
	x_timer = 0.0f;
	y_timer = 0.0f;
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

// Initialize list of characters for rendering
void Renderer::initializeCharacters()
{
	// Initiliaze FreeType library
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		cout << "Error: Unable to initlaize FreeType library" << endl;
		return;
	}

	// Get path to font
	string font_path = "./fonts/OpenSans-Regular.ttf";

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, font_path.c_str(), 0, &face)) 
	{
		cout << "Error: Unable to load font" << endl;
		return;
	}

	// Set global size and properties
	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Unpack first 128 characters of ASCII set into map
	for (unsigned char c = 0; c < 128; c++)
	{
		// Load character
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			cout << "Error: Unable to load character" << endl;
			continue;
		}

		// Generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		// Set texture properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		// Add character entry to map
		Character character = 
		{
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x)
		};

		Characters.insert(pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free reousrces
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

// Draw score on screen
void Renderer::drawScore(VertexArray& va, VertexBuffer& vb, Shader& shader, int score)
{
	// bind vertex array and shader
	va.bind();
	shader.bind();
	glActiveTexture(GL_TEXTURE0);

	// create and set projection matrix
	glm::mat4 projection = glm::ortho(0.0f, (float) WIDTH, 0.0f, (float) HEIGHT);
	shader.setUniform4Mat("projection", projection);

	// set text color and activate texture
	shader.setUniform3Vec("color", glm::vec3(1.0f, 1.0f, 1.0f));
	
	// set score text
	string text = "Score: 5";

	// initialize x and y coordinates
	float x = x_score;
	float y = y_score;

	// draw each character
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		// load character specific properties
		Character character = Characters[*c];

		// calculate x and y position of each character (incrementally)
		float xPos = x + character.bearing.x * scale;
		float yPos = y - (character.size.y - character.bearing.y) * scale;

		// calculate width and height of each character
		float charWidth = character.size.x * scale;
		float charHeight = character.size.y * scale;

		// update vertices for each character
		float vertices[6][4] = 
		{
			{ xPos,				yPos + charHeight,		0.0f, 0.0f },
			{ xPos,				yPos,					0.0f, 1.0f },
			{ xPos + charWidth, yPos,					1.0f, 1.0f },
			{ xPos,				yPos + charHeight,		0.0f, 0.0f },
			{ xPos + charWidth,	yPos,					1.0f, 1.0f },
			{ xPos + charWidth, yPos + charHeight,		1.0f, 0.0f }
		};

		// render texture over quad and bind buffer
		glBindTexture(GL_TEXTURE_2D, character.id);
		vb.bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// update advance offset
		x += (character.advance >> 6) * scale; // bitshift by 6 to get value in pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}