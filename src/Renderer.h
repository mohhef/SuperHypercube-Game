#pragma once
#include <GL/glew.h>
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include <map>

// Assertion check helper for gl commands
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) glClearError();\
    x;\
    ASSERT(glLogCall(#x, __FILE__, __LINE__))

// Empty error queue
void glClearError();
bool glLogCall(const char* function, const char* file, int line);

struct Character {
	unsigned int id; // texture id
	glm::ivec2   size;      
	glm::ivec2   bearing;   // offset from baseline
	unsigned int advance;   // offset to next character
};

class Renderer
{
private:
	// singleton instance
	static Renderer s_Instance;

	// models
	unsigned int renderIndex = 0;
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::vec3 centerOfMass;
	bool isFindingDepth = false;

	// characters
	map<GLchar, Character> Characters;
	float x_score;
	float y_score;
	float x_timer;
	float y_timer;
	float scale;

	Renderer();

	void initializeCharacters();

public:
	static Renderer& getInstance();
	Renderer(const Renderer&) = delete;
	bool isTextureEnabled = true;
	void setRenderIndex(unsigned int index);
	void setIsFindingDepth(bool findDepth);
	void updateCenterOfMass();

	// draw functions
	void clear() const;
	void drawAxes(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection);
	void drawLightingSource(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos);
	void drawObject(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, glm::mat4 modelRotMat, vector<glm::mat4> modelTransMat, float scaleFactor, glm::vec3 displacement);
	void drawWall(VertexArray & va, Shader & shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture, glm::mat4 modelRotMat, float scaleFactor, glm::vec3 displacement);
	void drawFloor(VertexArray& va, Shader& shader, glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 cameraPos, Texture& texture);
	void drawTimer(VertexArray& va, Shader& shader);
	void drawScore(VertexArray& va, VertexBuffer& vb, Shader& shader, int score);
};