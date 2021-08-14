//
// COMP 371 Assignment Framework
//
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.com/Getting-started/Hello-Triangle


#define GLEW_STATIC 1
#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>

#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DepthMapper.h"
#include "TextRendering.h"

// Trial log 1 # Tried reloading stb image
// Trial log 2 # Tried compiling stb.cpp
// Trial log 3 # Tried adding stb folder in lib
// Trial log 4 # Tried Updating stb header
// Trial log 5 # Every single error comes back to the stb image from Model
// fix circular dependencies
 #include "Model.h"


#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Constants.h"

#include "Renderer.h"
#include "stb_image.h" // Reference: Lab 4 (COMP 353) and LearnOpenGL; Source: https://github.com/nothings/stb/blob/master/stb_image.h
#include <irrklang/irrKlang.h>

using namespace irrklang;
using namespace std;

// Modified throughout run and to reset between runs.
// Possibly bound to a single model (modelIndex)
vector<glm::mat4> modelTransMat;
glm::mat4 modelRotMat;
glm::vec3 displacement;
float displacementSpeed = 1.0;
float scaleFactor = 1.0f;
bool textureStatus = true;
bool shadows = true;
int score = 0;
ISoundEngine* SoundEngine = createIrrKlangDevice();

// Cursor positions for mouse inputs
float lastMouseX;
float lastMouseY;

Camera* camera = NULL;

// function calls
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* initializeWindow();
void resetTransMat();
void resetRotMat(bool randomRot= false);
void resetModel(bool randomRot=false);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mode);
void processMouse(GLFWwindow* window, double xpos, double  ypos);
void createModel(vector<vector<int>> model);
void shuffleModel(vector<vector<int>> model);
void randomRotation();
void updateDisplacement(float currentFrame);
int getTotalCubes(vector <vector<int>> model);
bool isFit();

vector<vector<glm::vec3>> modelCubePositions;
vector<vector<glm::vec3>> wallCubePositions;

// Window size
int HEIGHT = 768;
int WIDTH = 1024;

// Cursor position parameters
float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;

// main function
int main(int argc, char* argv[])
{
	//create models
	for (auto &model : models) {
		createModel(model);
	}
	//TextRendering textRendering(WIDTH, HEIGHT);

	// SoundEngine->setSoundVolume(0.1f);
	// SoundEngine->play2D("audio/Kirby.mp3", true);

	GLFWwindow* window = initializeWindow();
	{
		// Setup for models
		VertexArray vA;
		VertexBuffer vB(vertices, sizeof(vertices));
		VertexBufferLayout layout;
		layout.push<float>(3);
		layout.push<float>(3);
		layout.push<float>(2);
		vA.addBuffer(vB, layout);
		
		// Setup for lighting
		VertexArray vaLightingSource;
		VertexBuffer vblightingSource(vertices, sizeof(vertices));
		VertexBufferLayout layoutLightingSource;
		layoutLightingSource.push<float>(3);
		layoutLightingSource.push<float>(3);
		layoutLightingSource.push<float>(2);
		vaLightingSource.addBuffer(vblightingSource, layoutLightingSource);

		// Setup for axes
		VertexArray vaAxes;
		VertexBuffer vbAxes(axesVertices, 3 * 2 * sizeof(float));
		VertexBufferLayout layoutAxes;
		layoutAxes.push<float>(3);
		vaAxes.addBuffer(vbAxes, layoutAxes);

		// Setup for floor
		VertexArray vaFloor;
		VertexBuffer vbFloor(squareVertices, sizeof(squareVertices));
		VertexBufferLayout layoutFloor;
		layoutFloor.push<float>(3);
		layoutFloor.push<float>(3);
		layoutFloor.push<float>(2);
		vaFloor.addBuffer(vbFloor, layoutFloor);

		// Create shader instances
		Shader* shader = new Shader("vertex_fragment.shader");
		Shader* axesShader = new Shader("axes.shader");
		Shader* lightingSourceShader = new Shader("lightingSource.shader");
		Shader* lightingShader = new Shader("lighting.shader");
		Shader* depthShader = new Shader("depthMap.shader");
		Shader* textShader = new Shader("text.shader");

		// telling the shader which textures go where
		shader->bind();
		shader->setUniform1i("textureObject", 0);
		shader->setUniform1i("depthMap", 1);
		textShader->bind();
		textShader->setUniform1i("text", 2);


		// Setup for shadows
		DepthMapper depthMapper;

		// Renddering setup
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);


		Renderer& renderer = Renderer::getInstance();

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);


		// Create camera instance
		// Position: behind model, along Z-axis.
		// Target: world origin (initially)
		camera = new Camera(glm::vec3(modelPosition.at(modelIndex).x +30, modelPosition.at(modelIndex).y +40, 100.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f));

		// Position of the light source
		glm::vec3 lightPos(0.0, 40.0f, 20.0f);

		// Initialize model matricies for each cube within each model 
		resetModel(true);
		glfwSetKeyCallback(window, processInput);
		glfwSetCursorPosCallback(window, processMouse);

		// load texture ids
		Texture brickTexture("brick.jpg");
		Texture tileTexture("tiles.jpg");
		Texture metalTexture("metal.jpg");


		// Entering main loop
		while (!glfwWindowShouldClose(window))
		{
			// Update last frame
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			updateDisplacement(currentFrame);

			// Clear color and depth buffers
			renderer.clear();

			glm::mat4 projection = glm::perspective(glm::radians(camera->zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 200.0f);
			glm::mat4 view = camera->getViewMatrix();

			// used to afterwards draw the shadows
			depthMapper.Draw(depthShader, lightPos, [&]() {
				// Render objects to be drawn by the depth mapper object
				renderer.drawObject(vA, *depthShader, view, projection, lightPos, camera->position, metalTexture, modelRotMat, modelTransMat, scaleFactor, displacement);
				renderer.drawWall(vA, *depthShader, view, projection, lightPos, camera->position, brickTexture, modelRotMat, scaleFactor, displacement);
				});

			// bind universal attributes necessary for drawing all the objects on the map
			shader->bind();
			shader->setUniform3Vec("lightPosition", lightPos);
			shader->setUniform3Vec("viewPos", camera->position);
			shader->setUniform1i("drawShadows", shadows);
			shader->setUniform1f("map_range", far);
			depthMapper.bind();

			// Render each object (wall, model, static models, axes, and mesh floor)
			renderer.updateCenterOfMass();
			renderer.drawObject(vA, *shader, view, projection, lightPos, camera->position, metalTexture, modelRotMat, modelTransMat, scaleFactor, displacement);
			renderer.drawWall(vA, *shader, view, projection, lightPos, camera->position, brickTexture, modelRotMat, scaleFactor, displacement);
			renderer.drawLightingSource(vaLightingSource, *lightingSourceShader, view, projection, lightPos);
			renderer.drawAxes(vaAxes, *axesShader, view, projection);
			
			
			//Position for the 3D model sample
			renderer.drawCube(vA, *lightingShader, view, projection);

			//Render the 3D
			renderer.drawModel(vA, *lightingShader, view, projection);

			// Render floor with tiles or draw the mesh depending on if we are drawing with or without textures
			renderer.drawFloor(vaFloor, *shader, view, projection, lightPos, camera->position, tileTexture);
			
			// Render light source
			renderer.drawLightingSource(vaLightingSource, *lightingSourceShader, view, projection, lightPos);
			
			glEnable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// Render text
			//textRendering.RenderText(*textShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			// End frame
			glfwSwapBuffers(window);

			// Detect inputs
			glfwPollEvents();
		}
	}

	// Shutdown GLFW
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	HEIGHT = height;
	WIDTH = width;
}

GLFWwindow* initializeWindow()
{
	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GL_DEPTH_BITS, 24);

	// Create Window and rendering context using GLFW, resolution is WIDTH x HEIGHT
	window = glfwCreateWindow(WIDTH, HEIGHT, "playground", NULL, NULL);
	if (!window)
	{
		cerr << "Failed to create GLFW window" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to create GLEW" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Background color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	return window;
}

// Wall check for score
// TODO: MAKE IT SO THAT IT CAN ACCEPT OTHER WAYS
bool isFit() 
{
	int numCubePieces = modelCubePositions.at(modelIndex).size();
	for (int i = 0; i < numCubePieces; i++)
	{
		int rows = models.at(modelIndex).size();
		int columns = models.at(modelIndex).at(0).size();

		glm::vec4 axes = glm::vec4(columns, rows, 1.0f, 1.0f);
		glm::mat4 model = glm::mat4(1.0f) * modelRotMat;
		glm::vec4 cubePos = model * glm::vec4(modelCubePositions.at(modelIndex).at(i), 1.0f);

		cubePos.x = round(cubePos.x);
		cubePos.y = round(cubePos.y);

		if (cubePos.x != modelCubePositions.at(modelIndex).at(i).x
			|| cubePos.y != modelCubePositions.at(modelIndex).at(i).y)
		{
			return false;
		}
	}
	
	return true;
}

//update the displacement of the object
void updateDisplacement(float currentFrame) {
	//update z displacement
	displacement.z = currentFrame * -1 * displacementSpeed;
	//if passes the wall reset
	if (displacement.z < -30) {
		//if the object fits in wall, increment score
		if (isFit()) 
		{
			score += 1;
			
			// NOTE: CURRENTLY PRINTING HERE TO SHOW YOU THAT IT WORKS
			cout << score << endl;
		}
		modelIndex = (modelIndex+1) % models.size();
		Renderer::getInstance().setRenderIndex(modelIndex);
		
		//resset time, z dispalcement and speed
		glfwSetTime(0);
		displacement.z = 0;
		displacementSpeed = 1;
		
		resetModel();
		randomRotation();
	}
}

// Reset translation matrix for each model's cube.
void resetTransMat()
{
	modelTransMat.resize(modelCubePositions.at(modelIndex).size());
	for (int i = 0; i < modelCubePositions.at(modelIndex).size(); i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, modelCubePositions.at(modelIndex).at(i));
		modelTransMat.at(i) = model;
	}
}

// Reset model's rotation matrix.
void resetRotMat(bool randomRot)
{
	modelRotMat = glm::mat4(1.0f);

	if (randomRot) {
		randomRotation();
	}
}

// Reset model to initial position
void resetModel(bool randomRot)
{
	resetTransMat();
	resetRotMat(randomRot);
	glfwSetTime(0.0f);
	displacement = glm::vec3();
	scaleFactor = 1.0f;
}

// Handle all keyboard inputs
void processInput(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Closes window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// Camera rotation around world axis (UP/DOWN/LEFT/RIGHT)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera->processMovement(KEY::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera->processMovement(KEY::DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera->processMovement(KEY::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera->processMovement(KEY::RIGHT, deltaTime);

	// Camera reset (HOME)
	if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS)
		camera = new Camera(glm::vec3(modelPosition.at(modelIndex).x, modelPosition.at(modelIndex).y, 100.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f));

	// Reset model (SPACEBAR)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		resetModel();

	// Scale model and wall (U/J)
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && scaleFactor < 1.25f)
		scaleFactor += 0.01f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && scaleFactor > 0.75f)
		scaleFactor -= 0.01f;

	// Model displacement (W/S/A/D) and rotation (w/s/a/d), the latter of which around it's own axis.
	if (mode == GLFW_MOD_SHIFT) {
		displacementSpeed *= 1.5f;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelRotMat= model * modelRotMat;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelRotMat = model * modelRotMat;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelRotMat = model * modelRotMat;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelRotMat = model * modelRotMat;
	}
	if (key == GLFW_KEY_Q)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelRotMat= model * modelRotMat;
	}
	if (key == GLFW_KEY_E)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelRotMat = model * modelRotMat;
	}
	// Toggle rendering mode between point, line and fill mode (P/L/T)
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Shuffle models
	if (key == GLFW_KEY_Y) 
	{
		shuffleModel(models.at(modelIndex));
		resetModel(true);
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		shadows = !shadows;
	}
}

// Function for processing mouse input
void processMouse(GLFWwindow* window, double xpos, double  ypos)
{
	// Required for continous call (holding down button)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE
		&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE
		&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
	{
		initialMousePos = true;
		return;
	}

	// Required to prevent jump when toggling mouse button
	// Note that call can be made from different cursor position
	if (initialMousePos)
	{
		lastMouseX = xpos;
		lastMouseY = ypos;
		initialMousePos = false;
	}

	// Camera zoom (LMB)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		float yOffset = lastMouseY - ypos;

		lastMouseX = xpos;
		lastMouseY = ypos;

		camera->zoomCamera(yOffset);
	}

	// Camera pan (RMB)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		float xOffset = lastMouseX - xpos;

		lastMouseX = xpos;
		lastMouseY = ypos;

		camera->panCamera(xOffset);
	}

	// Camera tilt (MMB)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		float yOffset = lastMouseY - ypos;

		lastMouseX = xpos;
		lastMouseY = ypos;

		camera->tiltCamera(yOffset);
	}
}

void createModel(vector<vector<int>> model) {
	int rows = model.size();
	vector<glm::vec3> wallPos;
	vector<glm::vec3> modelPos;
	float z = -10.0f;
	//start from bottom left
	for (int i = rows-1; i > -1; i--) {
		int cols = model.at(i).size();
		for (int j = 0; j < cols; j++) {
			if (model.at(i).at(j) == 0) {
				//if 0 then wall cube
				wallPos.push_back(glm::vec3(float(j), float(abs(i - (rows - 1))), float(z)));
			}
			else {
				//if +ve the stack along +ve Z axis, otherwise +ve z
				int multiplyer = 1.0f;
				if (model.at(i).at(j) < 0) {
					multiplyer = -1.0f;
				}
				//stack cubes along z axis
				for (int zStack = 0; zStack < abs(model[i][j]); zStack++) {
					modelPos.push_back(glm::vec3(float(j), float(abs(i - (rows - 1))), float(zStack * multiplyer)));
				}
			}
		}
	}
	modelCubePositions.push_back(modelPos);
	wallCubePositions.push_back(wallPos);
};

void shuffleModel(vector<vector<int>> model) {
	srand(time(0));
	//Max number of cubes
	int remainingCube = getTotalCubes(model);

	int rows = model.size();

	vector<glm::vec3> modelPos;
	for (int i = rows - 1; i > -1; i--) {
		int cols = model.at(i).size();
		for (int j = 0; j < cols; j++) {
			if (model.at(i).at(j) != 0) {
				if (remainingCube == 0) {
					break;
				}
				//for z-axis, between -3 -> +3
				int numCube = 0;
				while (numCube == 0) {
					numCube = rand() % 7 + (-3);
					if (abs(numCube) > remainingCube) {
						numCube = remainingCube;
					}
				}
				remainingCube -= abs(numCube);

				//if -ve z or +ve z axis
				int multiplyer = 1.0f;
				if (numCube < 0) {
					multiplyer = -1.0f;
				}
				// stack the cubes along z
				for (int zStack = 0; zStack < abs(numCube); zStack++) {
					modelPos.push_back(glm::vec3(float(j), float(abs(i - (rows - 1))), float(zStack * multiplyer)));
				}
			}
		}
	}

	modelCubePositions.at(modelIndex) = modelPos;
}

//rotate a model randomly when shuffling
void randomRotation() {
	glm::mat4 randRotMat = glm::mat4(1.0f);
	//random rotation angle from -100 -> 100, multiple of 10
	float rotAng = (rand() % (4 - 0 + 1) + 0)*90.0f;
	//random x rotation
	randRotMat = glm::rotate(randRotMat, glm::radians(rotAng), glm::vec3(1.0f, 0.0f, 0.0f));
	//random y rotation
	rotAng = (rand() % (4 - 0 + 1) + 0) * 90.0f;
	randRotMat = glm::rotate(randRotMat, glm::radians(rotAng), glm::vec3(0.0f, 1.0f, 0.0f));
	//random z rotation
	rotAng = (rand() % (4 - 0 + 1) + 0) * 90.0f;
	randRotMat = glm::rotate(randRotMat, glm::radians(rotAng), glm::vec3(0.0f, 0.0f, 1.0f));

	modelRotMat = randRotMat;
}

//count the total number of cubes in a model
int getTotalCubes(vector<vector<int>> model) {
	int total = 0;
	for (auto& row : model) {
		for (auto& col : row) {
			total += abs(col);
		}
	}
	return total;
}