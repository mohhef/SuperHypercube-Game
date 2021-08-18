//
// COMP 371 Assignment Framework
//
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.com/Getting-started/Hello-Triangle


#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctime>

#include <iostream>
#include <string>

#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DepthMapper.h"
#include "TextRendering.h"
#include "Animations.h"

#include "Model.h"
#include "ModelShader.h"
#include "Mesh.h"

#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Constants.h"

#include "Renderer.h"
#include "stb_image.h" // Reference: Lab 4 (COMP 353) and LearnOpenGL; Source: https://github.com/nothings/stb/blob/master/stb_image.h
#include <irrklang/irrKlang.h>

using namespace irrklang;
using namespace std;

// Score multiplier variables
int scoreMultiplier = 1;
int multiplierCounter = 0;
int incMultiplierThreshold = 2;
// collision variables
float prevDisplacementSpeed = 0.0f;
bool displacementSpeedUpdate = true;
float speedAfterPass = 6.6f;
glm::vec3 lockedCameraLookAt = glm::vec3(1.0f);
// Modified throughout run and to reset between runs.
// Possibly bound to a single model (modelIndex)
// Models
vector<glm::mat4> modelTransMat;
RotationMat rotMat(glm::vec3(0.0f));
glm::vec3 displacement;
float displacementSpeed = 2.0;
float scaleFactor = 1.0f;
bool textureStatus = true;
vector<vector<glm::vec3>> modelCubePositions;
vector<vector<glm::vec3>> wallCubePositions;

// Shadows
bool shadows = true;

// Text
int score = 0;
int wallsCleared = 0;
int numCubes = 0;
clock_t timer;

// Renderer
Renderer& renderer = Renderer::getInstance();

// Sound
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundEngine* SoundEngine2 = createIrrKlangDevice();
float bgmVolume = 0.25f;
float effectVolume = 0.50f;
int lastMinusState = GLFW_RELEASE;
int lastEqualState = GLFW_RELEASE;
int lastBackSpaceState = GLFW_RELEASE;
int soundSwitch = 1;
// Cursor positions for mouse inputs
float lastMouseX;
float lastMouseY;

// Camera
Camera* camera = NULL;

// Function calls
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* initializeWindow();
void resetTransMat();
void resetRotMat(bool randomRot= false);
void resetModel(bool randomRot=false);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mode);
void processMouse(GLFWwindow* window, double xpos, double  ypos);
void createModel(vector<vector<int>> model);
void randomRotation();
void updateDisplacement(float currentFrame);
void updateNumberOfCubes();
int getTotalCubes(vector <vector<int>> model);
bool isFit();
void resetScoreAndTimer();

// animation
bool resetAfterCollision = false;
bool fittingThrough = false;
float resetTime = 0.0f;
glm::vec3 cameraInitialPos = glm::vec3(modelPosition.at(modelIndex).x, modelPosition.at(modelIndex).y + 40, 100.0f);
glm::vec3 modelPos;

// Window size
int HEIGHT = 768;
int WIDTH = 1024;

// Cursor position parameters
float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;


// Main function
int main(int argc, char* argv[])
{
	// Seed random generator
	srand(time(NULL));

	// Create models
	for (auto &model : models) {
		createModel(model);
	}


	SoundEngine->setSoundVolume(bgmVolume);
	SoundEngine->play2D("audio/Kirby.mp3",  true, false, true);

	SoundEngine2->setSoundVolume(effectVolume);
	SoundEngine2->addSoundSourceFromFile("audio/breakout.mp3", ESM_AUTO_DETECT, true); // third parameter set to true == preload
	SoundEngine2->addSoundSourceFromFile("audio/punch.mp3", ESM_AUTO_DETECT, true); // third parameter set to true == preload
	SoundEngine2->addSoundSourceFromFile("audio/bow.mp3", ESM_AUTO_DETECT, true); // third parameter set to true == preload
	SoundEngine2->addSoundSourceFromFile("audio/score.wav", ESM_AUTO_DETECT, true); // third parameter set to true == preload
	SoundEngine2->addSoundSourceFromFile("audio/click.mp3", ESM_AUTO_DETECT, true); // third parameter set to true == preload

	GLFWwindow* window = initializeWindow();
	{
		TextRendering textRendering(WIDTH, HEIGHT);

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
		Shader* depthShader = new Shader("depthMap.shader");
		Shader* textShader = new Shader("text.shader");

		// 3D Models
		ModelShader* d3Shader = new ModelShader("3DmodelVertex.shader", "3DmodelFragment.shader");
		Model ivysaurmodel("3D_Model/pokemon.obj");   // ivysaur
		Model charizardmodel("3D_Model/pokemon1.obj"); // charizard
		Model squirtlemodel("3D_Model/pokemon2.obj"); // squirtle

		// telling the shader which textures go where
		shader->bind();
		shader->setUniform1i("textureObject", 0);
		shader->setUniform1i("depthMap", 1);
		textShader->bind();
		textShader->setUniform1i("text", 2);

		// Setup for shadows
		DepthMapper depthMapper;

		// Renddering setup
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Create camera instance
		// Position: behind model, along Z-axis.
		// Target: world origin (initially)
		camera = new Camera(glm::vec3(modelPosition.at(modelIndex).x, modelPosition.at(modelIndex).y + 40, 100.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 20.0f, 0.0f));

		// Position of the light source
		glm::vec3 lightPos(-10.0, 40.0f, 40.0f);

		modelPos = (modelPosition.at(modelIndex) * scaleFactor) + modelCenter.at(modelIndex) + glm::vec3(1.0f, modelHeight + 1.0f, 0.0f);

		// Initialize model matricies for each cube within each model 
		resetModel(true);
		glfwSetKeyCallback(window, processInput);
		glfwSetCursorPosCallback(window, processMouse);

		updateNumberOfCubes();

		// Load texture ids
		Texture tetrisTexture("texture/tetris.jpg");
		Texture galaxyTexture("texture/galaxy.jpg");
		Texture brickTexture("texture/white_brick.jpg");

		// Set timer
		timer = clock();

		// Entering main loop
		while (!glfwWindowShouldClose(window))
		{	
			SoundEngine->setSoundVolume(bgmVolume);
			//SoundEngine->play2D("audio/Kirby.mp3", true);
			
			// Update last frame
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			if (resetAfterCollision && currentFrame > resetTime) {
				resetAfterCollision = false;
				resetTime = 0.0f;
				resetModel(true);
				// handle multiplier
				multiplierCounter = 0;
				// reduce multipler to half
				if (scoreMultiplier > 1)scoreMultiplier = scoreMultiplier / 2;
			}

			if (resetAfterCollision) {
				camera->lookAt(modelPos + displacement);
			}

			rotMat.updateRotation(currentFrame);

			updateDisplacement(currentFrame);
			renderer.updateCenterOfMass();

			// Clear color and depth buffers
			renderer.clear();

			glm::mat4 projection = glm::perspective(glm::radians(camera->zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 600.0f);
			glm::mat4 view = camera->getViewMatrix();

			// Shadow mapping
			depthMapper.Draw(depthShader, lightPos, [&]() {
				// Render objects to be drawn by the depth mapper object
				renderer.drawObject(vA, *depthShader, view, projection, lightPos, camera->position, tetrisTexture, rotMat.getMatrix(), modelTransMat, scaleFactor, displacement);
				renderer.drawWall(vA, *depthShader, view, projection, lightPos, camera->position, brickTexture, rotMat.getMatrix(), scaleFactor, displacement, resetAfterCollision, fittingThrough);
				});

			// Bind universal attributes necessary for drawing all the objects on the map
			shader->bind();
			shader->setUniform3Vec("lightPosition", lightPos);
			shader->setUniform3Vec("viewPos", camera->position);
			shader->setUniform1i("drawShadows", shadows);
			shader->setUniform1f("map_range", far);
			depthMapper.bind();

			// Render each object (wall, model, static models, axes, and mesh floor)
			renderer.drawObject(vA, *shader, view, projection, lightPos, camera->position, tetrisTexture, rotMat.getMatrix(), modelTransMat, scaleFactor, displacement);
			
			renderer.drawWall(vA, *shader, view, projection, lightPos, camera->position, brickTexture, rotMat.getMatrix(), scaleFactor, displacement, resetAfterCollision, fittingThrough);
			renderer.drawLightingSource(vaLightingSource, *lightingSourceShader, view, projection, lightPos);
			renderer.drawFloor(vaFloor, *shader, view, projection, lightPos, camera->position, galaxyTexture);

			// Development purpose
			// renderer.drawAxes(vaAxes, *axesShader, view, projection);	

			// Draw Ivysaur
			renderer.draw3DModel(
				*d3Shader,
				view,
				projection,
				glm::vec3(2.0f, 2.0f, 2.0f),
				glm::vec3(8.0f, 20.0f, -10.0f),
				glm::vec3(0.0f, 135.0f, 0.0f),
				ivysaurmodel
			);

			// Draw Charizard
			renderer.draw3DModel(
				*d3Shader,
				view,
				projection,
				glm::vec3(0.2f, 0.2f, 0.2f),
				glm::vec3(4.0f, 20.0f, -10.0f),
				glm::vec3(10.0f, 0.0f, 10.0f),
				charizardmodel
			);

			// Draw Squirtle
			renderer.draw3DModel(
				*d3Shader,
				view,
				projection,
				glm::vec3(0.7f, 0.7f, 0.7f),
				glm::vec3(-2.5f, 20.0f, -12.5f),
				glm::vec3(0.0f, 70.0f, 0.0f),
				squirtlemodel
			);
			
			// Render light source
			renderer.drawLightingSource(vaLightingSource, *lightingSourceShader, view, projection, lightPos);
			
			// Update timer
			int totalSeconds = 120 - (int) ((clock() - timer) / (double) CLOCKS_PER_SEC);

			if (totalSeconds < 0)
				resetScoreAndTimer();

			int minutes = totalSeconds / 60;
			int seconds = totalSeconds % 60;

			// Render text
			textRendering.enable();
			textRendering.RenderText(*textShader, "Score: " + to_string(score), 50.0f, 700.0f, 0.50f, modelColor.at(modelIndex));
			textRendering.RenderText(*textShader, "Multiplier: " + to_string(scoreMultiplier), 50.0f, 670.0f, 0.50f, modelColor.at(modelIndex));
			textRendering.RenderText(*textShader, "Walls cleared : " + to_string(wallsCleared), 50.0f, 640.0f, 0.50f, modelColor.at(modelIndex));
			textRendering.RenderText(*textShader, "Number of cubes in cluster : " + to_string(numCubes), 50.0f, 610.0f, 0.50f, modelColor.at(modelIndex));
			if (seconds < 10)
				textRendering.RenderText(*textShader, "Time: " + to_string(minutes) + ":0" + to_string(seconds), 850.0f, 700.0f, 0.50f, modelColor.at(modelIndex));
			else
				textRendering.RenderText(*textShader, "Time: " + to_string(minutes) + ":" + to_string(seconds), 850.0f, 700.0f, 0.50f, modelColor.at(modelIndex));
			textRendering.disable();

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

// Callback for window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	HEIGHT = height;
	WIDTH = width;
}

// Initialize window function
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	return window;
}

// Wall check for score
bool isFit() 
{
	int numCubePieces = modelCubePositions.at(modelIndex).size();
	for (int i = 0; i < numCubePieces; i++)
	{
		int rows = models.at(modelIndex).size();
		int columns = models.at(modelIndex).at(0).size();

		//glm::vec4 axes = glm::vec4(columns, rows, 1.0f, 1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glm::vec4 cubePos = model
			* glm::translate(glm::mat4(1.0f), renderer.centerOfMass)
			* rotMat.getMatrix()
			* glm::translate(glm::mat4(1.0f), -renderer.centerOfMass)
			* glm::vec4(modelCubePositions.at(modelIndex).at(i), 1.0f);

		cubePos.x = round(abs( cubePos.x));
		cubePos.y = round(abs(cubePos.y));

		bool foundPlace = false;
		for (int x = 0; x < numCubePieces; x++) {
			if (cubePos.x == modelCubePositions.at(modelIndex).at(x).x && cubePos.y == modelCubePositions.at(modelIndex).at(x).y) {
				foundPlace = true;
				break;
			}
		}

		if (!foundPlace) {
			return false;
		}
	}
	
	return true;
}

//update the displacement of the object
void updateDisplacement(float currentFrame) 
{
	static float prevFrame = currentFrame;
	// Update z displacement
	if (!resetAfterCollision) {
		displacement.z += (prevFrame - currentFrame) * displacementSpeed;
		camera->moveCamera(glm::vec3(0.0f, 0.0f, (prevFrame - currentFrame) * displacementSpeed));
	}
	else {
		displacement.y += (prevFrame - currentFrame) * 20.0f;
	}
	
	prevFrame = currentFrame;
	
	// Check for collision
	if (renderer.calculateFurthestZ(rotMat.getMatrix(), modelTransMat, displacement) < wallZPos + 2)
		if (!isFit() && !resetAfterCollision) {
			SoundEngine2->play2D("audio/punch.mp3", false);
			// random rotation and x movement
			float LO = -1.0f;
			float HI = 1.0f;
			float r = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			rotMat.setSoft(glm::vec3(0.0f, 0.0f, r * 20.0f));
			displacement.x += 1.3 * r;
			resetTime = currentFrame + 0.5f;
			resetAfterCollision = true;
			fittingThrough = false;
		}	
		// camera lookAt does not move forward, slows down when passed
		else if (displacement.z < -18 && displacement.z > -25){
			if (displacementSpeedUpdate) {
				if (displacementSpeed >= 10.0f) { prevDisplacementSpeed = speedAfterPass; }
				else {
					prevDisplacementSpeed = displacementSpeed;
				}
				displacementSpeedUpdate = false;
				lockedCameraLookAt = (modelPos + displacement);
			}
			displacementSpeed = prevDisplacementSpeed;
			camera->lookAt(lockedCameraLookAt);
			if (isFit()) fittingThrough = true;
		}
	// Reset upon wall pass
		else if (displacement.z < -25) {
			// Increment if model fits through hole
			if (isFit()) {
				SoundEngine2->play2D("audio/score.wav", false);
				score += 1 * scoreMultiplier;
				multiplierCounter++;
				wallsCleared += 1;
			}
			// update multiplier upon threshold
			if (multiplierCounter >= incMultiplierThreshold) {
				scoreMultiplier += 1;
				multiplierCounter = 0;
			}

			modelIndex = (modelIndex + 1) % models.size();
			Renderer::getInstance().setRenderIndex(modelIndex);

			// Z displacement and speed
			displacement.z = 0;

			displacementSpeedUpdate = true;
			resetModel();
			randomRotation();
			updateNumberOfCubes();
		}
		else {
			displacementSpeed = 10.0f;
			camera->lookAt(modelPos + displacement);
		}
}

// Update the number of cubes in cluster based on current model
void updateNumberOfCubes()
{
	numCubes = getTotalCubes(models.at(modelIndex));
	fittingThrough = false;
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
	rotMat.reset();

	if (randomRot) {
		randomRotation();
	}
}

// Reset model to initial position
void resetModel(bool randomRot)
{
	resetTransMat();
	resetRotMat(randomRot);
	glm::vec3 cameraPos = modelPos + glm::vec3(0.0f, 10.0f, 40.0f);
	camera->resetPos(cameraPos);
	camera->lookAt(cameraPos + glm::vec3(0.0f, -10.0f, -50.0f));
	glfwSetTime(0.0f);
	displacement = glm::vec3(0.0f);
	displacementSpeed = 2.0f;
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
	if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
		glm::vec3 cameraPos = modelPos + glm::vec3(0.0f, 0.0f, 40.0f);
		camera->resetPos(cameraPos + displacement);
		camera->lookAt(cameraPos + glm::vec3(0.0f, 0.0f, -50.0f));
	}

	// Reset model (SPACEBAR)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		resetModel();

	// Model displacement (W/S/A/D) and rotation (w/s/a/d), the latter of which around it's own axis.
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		displacementSpeed = 60.0f;
	}

	// rotation movemement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(-90.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(90.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(0.0f, 90.0f, 0.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(0.0f, -90.0f, 0.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(0.0f, 0.0f, 90.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		SoundEngine2->play2D("audio/click.mp3", false);
		rotMat.setSoft(glm::vec3(0.0f, 0.0f, -90.0f));
	}
	// Toggle rendering mode between point, line and fill mode (P/L/T)
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		shadows = !shadows;
	}
	// bgm sound adjust
	if (lastMinusState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
		bgmVolume -= 0.05;
	}
	lastMinusState = glfwGetKey(window, GLFW_KEY_MINUS);
	if (lastEqualState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
		bgmVolume += 0.05;
	}
	lastEqualState = glfwGetKey(window, GLFW_KEY_EQUAL);
	if (lastBackSpaceState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		SoundEngine->stopAllSounds();
		switch (soundSwitch) {
		case 0:
			SoundEngine->play2D("audio/Kirby.mp3", true, false, true);
			break;
		case 1:
			SoundEngine->play2D("audio/breakout.mp3", true, false, true);
			break;
		case 2:
			SoundEngine->play2D("audio/salt.mp3", true, false, true);
			break;
		}
		soundSwitch++;
		if (soundSwitch >= 3) soundSwitch = 0;
	}
	lastBackSpaceState = glfwGetKey(window, GLFW_KEY_BACKSPACE);
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

// Create models for each object
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

// Rotate a model randomly when shuffling
void randomRotation() 
{
	float xRot = rand() % 3;
	float yRot = rand() % 3;
	float zRot = rand() % 3;

	rotMat.setHard(glm::vec3(xRot * 90, yRot * 90, zRot * 90));
}

// Count the total number of cubes in a model
int getTotalCubes(vector<vector<int>> model) {
	int total = 0;
	for (auto& row : model) {
		for (auto& col : row) {
			total += abs(col);
		}
	}
	return total;
}

// Reset score and timer when time ends
void resetScoreAndTimer()
{
	score = 0;
	timer = clock();
}
