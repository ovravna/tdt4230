
#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "gametools.hpp"
#include "sceneGraph.hpp"

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "camera.hpp"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
glm::mat4 orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));
glm::mat4 view;


glm::vec4 lights[3];
glm::vec3 lightColors[3] {
	glm::vec3(1, 1, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 0, 1),

};

int lightIdx = 0;
GLint normalMatricLoc;
GLint mv3x3Loc;
GLint camLoc;
GLint ballLoc;
GLint lightSourcesLoc;
GLint drawModeLoc;

GLint orthoProjectionLoc;
GLint textPosLoc;

Camera * cam;

Mesh text;

SceneNode* rootNode;

SceneNode* textNode;


double ballRadius = 3.0f;
double timeDelta;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shaderText;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 cameraPosition = glm::vec3(0, 2, -20);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;
bool isPaused = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;

void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

SceneNode * newBall(SceneNode * parent, float radius, float slices, float layers, glm::vec3 position = glm::vec3(0), SceneNodeType nodeType = GEOMETRY, glm::vec3 referencePoint = glm::vec3(0), glm::vec3 rotation = glm::vec3(0))  {
	SceneNode * node = createSceneNode();
	node->nodeType = nodeType;
	node->position = position;
	node->referencePoint = referencePoint;
	node->rotation = rotation;
	node->currentTransformationMatrix = glm::mat4(1);
	
    Mesh box = generateSphere(radius, slices, layers);
    node->vertexArrayObjectID = generateBuffer(box);
	node->VAOIndexCount = box.indices.size();

	parent->children.push_back(node);

	return node;

}

SceneNode * newBox(SceneNode * parent, glm::vec3 dimentions, glm::vec3 position = glm::vec3(0), Mesh * mesh = nullptr, glm::vec4 color = glm::vec4(1), SceneNodeType nodeType = GEOMETRY, glm::vec3 referencePoint = glm::vec3(0), glm::vec3 rotation = glm::vec3(0))  {

	SceneNode * node = createSceneNode();
	node->nodeType = nodeType;
	node->position = position;
	node->referencePoint = referencePoint;
	node->rotation = rotation;
	node->currentTransformationMatrix = glm::mat4(1);
	node->color = color;
	
	/* if (mesh == nullptr) */ 
	/* 	*mesh = cube(dimentions); */

	Mesh box = cube(dimentions);
    node->vertexArrayObjectID = generateBuffer(box);
	node->VAOIndexCount = box.indices.size();

	parent->children.push_back(node);

	return node;

}


SceneNode * initText(SceneNode * parent) {

	PNGImage charmap = loadPNGFile("res/textures/charmap.png");
	/* PNGImage diffuseTexture = loadPNGFile("res/textures/Brick03_col.png"); */
	/* PNGImage normalMap = loadPNGFile("res/textures/Brick03_nrm.png"); */
	/* PNGImage roughnessMap = loadPNGFile("res/textures/Brick03_rgh.png"); */

	text = generateTextGeometryBuffer(30); 

	textNode = createSceneNode();
	textNode->vertexArrayObjectID = generateBuffer(text);
	textNode->nodeType = GEOMETRY_2D;
	textNode->textureID = loadTextureFromImage(charmap);
	textNode->VAOIndexCount = text.indices.size();

	textNode->position = glm::vec3(10, windowHeight - 50, 0);

	addChild(parent, textNode);

	return textNode;
}

SceneNode * newLight(SceneNode * parent, glm::vec3 position) {

	SceneNode * lightNode = createSceneNode();
	lightNode->nodeType = POINT_LIGHT;
	lightNode->position = position;
	addChild(parent, lightNode);
	return lightNode;
}

//// A few lines to help you if you've never used c++ structs



void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("res/Hall of the Mountain King.ogg")) {
        return;
    }
    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

	cam = new Camera(window);
	cam->position = glm::vec3(5, 1.8, 5);

    shader = new Gloom::Shader();
    shader->makeBasicShader("res/shaders/simple.vert", "res/shaders/simple.frag");

    shaderText = new Gloom::Shader();
    shaderText->makeBasicShader("res/shaders/text.vert", "res/shaders/text.frag");
	

    shader->activate();

	normalMatricLoc = glad_glGetUniformLocation(shader->get(), "normalMatrix");
	mv3x3Loc = glad_glGetUniformLocation(shader->get(), "MV3x3");
	camLoc = glad_glGetUniformLocation(shader->get(), "camPos");
	ballLoc = glad_glGetUniformLocation(shader->get(), "ballPos");
	lightSourcesLoc = glad_glGetUniformLocation(shader->get(), "lightSources");
	drawModeLoc = glad_glGetUniformLocation(shader->get(), "drawMode");

	orthoProjectionLoc = glad_glGetUniformLocation(shaderText->get(), "orthoProjection");
	textPosLoc = glad_glGetUniformLocation(shaderText->get(), "textPos");



    // Construct scene
    rootNode = createSceneNode();

	initText(rootNode);
    /* newBall(rootNode, 1.0, 40, 40, glm::vec3(0, 0, 1)); */
	/* newBox(rootNode, glm::vec3(1), glm::vec3(0, 0, 4)); */

	int size = 10;
	for (int y = 0; y < size; y++) 
		for (int x = 0; x < size; x++) {
			newBox(rootNode, glm::vec3(1), glm::vec3(x, 0, y));
			newBox(rootNode, glm::vec3(1), glm::vec3(size, x, y));
			newBox(rootNode, glm::vec3(1), glm::vec3(0, x, y));

		}

	newBox(rootNode, glm::vec3(1), glm::vec3(3, 1, 3), nullptr, glm::vec4(1, 0, 0, 1));  

	 
	newLight(rootNode, glm::vec3(0, 30, 0));


    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}


void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    timeDelta = getTimeDeltaSeconds();

    updateNodeTransformations(rootNode, glm::mat4(1));

}

void renderNode(SceneNode* node) {
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

	auto nm = glm::mat3(glm::transpose(glm::inverse(node->currentTransformationMatrix)));
	auto mv3x3 = glm::mat3(cam->getView() * node->currentTransformationMatrix);

    glUniformMatrix3fv(normalMatricLoc, 1, GL_FALSE, glm::value_ptr(nm));
    glUniformMatrix3fv(mv3x3Loc, 1, GL_FALSE, glm::value_ptr(mv3x3));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {

				glad_glUniform1i(drawModeLoc, 0);
    			glUniform4fv(12, 1, glm::value_ptr(node->color));
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
		case GEOMETRY_NORMAL_MAPPED: 
			if (node->vertexArrayObjectID != -1) {
				glad_glUniform1i(drawModeLoc, 2);

				glBindTextureUnit(1, node->textureID);
				glBindTextureUnit(2, node->normalMapTextureID);
				glBindTextureUnit(3, node->roughnessID);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

				glad_glUniform1i(drawModeLoc, 0);
			}

			break;
		case GEOMETRY_2D:
			break;
        case POINT_LIGHT: 
			lights[lightIdx++] = node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1);
			break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderText(SceneNode * node) {
	
		shaderText->activate();
		glBindTextureUnit(0, node->textureID);

		int fps = std::round(1.0f / timeDelta);
		setText(&text, fmt::format("{}", fps) );
		/* setText(&text, glm::to_string(cam->front) ); */

		updateTextureCoordinates(node->vertexArrayObjectID, text);

		/* glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(view)); */
		/* glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(projection)); */
		/* glUniformMatrix3fv(orthoProjectionLoc, 1, GL_FALSE, glm::value_ptr(nm)); */
		glUniformMatrix4fv(orthoProjectionLoc, 1, GL_FALSE, glm::value_ptr(orthoProjection));
		glUniform3fv(textPosLoc, 1, glm::value_ptr(node->position));
		
		/* glBindTexture(GL_TEXTURE_2D, node->textureID); */ 
		/* node->currentTransformationMatrix *= glm::scale(glm::vec3(0.3f, 0.3f, 0.3f)); */
		glBindVertexArray(node->vertexArrayObjectID);
		glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

		shader->activate();
}


void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    glUniformMatrix4fv(4, 1, GL_FALSE, cam->getViewPtr());
    glUniformMatrix4fv(5, 1, GL_FALSE, cam->getProjectionPtr());

    glUniform4fv(6, 1, glm::value_ptr(lights[0]));
    glUniform4fv(7, 1, glm::value_ptr(lights[1]));
    glUniform4fv(8, 1, glm::value_ptr(lights[2]));

    glUniform3fv(9, 1, glm::value_ptr(lightColors[0]));
    glUniform3fv(10, 1, glm::value_ptr(lightColors[1]));
    glUniform3fv(11, 1, glm::value_ptr(lightColors[2]));

	glUniform3fv(ballLoc, 1, glm::value_ptr(ballPosition));

	lightIdx = 0;
    renderNode(rootNode);
	renderText(textNode);

}


void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
	cam->handleKeyboardInput();
}
