
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

glm::mat4 projection, view;
glm::mat4 orthoProjection;

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
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;

SceneNode* lightNode1;
SceneNode* lightNode2;
SceneNode* lightNode3;

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

SceneNode * newBox(SceneNode * parent, glm::vec3 dimentions, SceneNodeType nodeType = GEOMETRY, glm::vec3 position = glm::vec3(0), glm::vec3 referencePoint = glm::vec3(0), glm::vec3 rotation = glm::vec3(0))  {

	SceneNode * node = createSceneNode();
	node->nodeType = nodeType;
	node->position = position;
	node->referencePoint = referencePoint;
	node->rotation = rotation;
	node->currentTransformationMatrix = glm::mat4(1);
	
    Mesh box = cube(dimentions);
    node->vertexArrayObjectID = generateBuffer(box);
	node->VAOIndexCount = box.indices.size();

	parent->children.push_back(node);

	return node;

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
	/* cam->position = cameraPosition; */

    shader = new Gloom::Shader();
    shader->makeBasicShader("res/shaders/simple.vert", "res/shaders/simple.frag");

    shaderText = new Gloom::Shader();
    shaderText->makeBasicShader("res/shaders/text.vert", "res/shaders/text.frag");
	

    shader->activate();

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
	textNode->position = glm::vec3(0);

	normalMatricLoc = glad_glGetUniformLocation(shader->get(), "normalMatrix");
	mv3x3Loc = glad_glGetUniformLocation(shader->get(), "MV3x3");
	camLoc = glad_glGetUniformLocation(shader->get(), "camPos");
	ballLoc = glad_glGetUniformLocation(shader->get(), "ballPos");
	lightSourcesLoc = glad_glGetUniformLocation(shader->get(), "lightSources");
	drawModeLoc = glad_glGetUniformLocation(shader->get(), "drawMode");

	orthoProjectionLoc = glad_glGetUniformLocation(shaderText->get(), "orthoProjection");
	textPosLoc = glad_glGetUniformLocation(shaderText->get(), "textPos");

	orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));
    projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    // Create meshes
    /* Mesh pad = cube(padDimensions, glm::vec2(30, 40), true); */
    /* Mesh box = cube(boxDimensions, glm::vec2(90), true, false); */
    Mesh sphere = generateSphere(1.0, 40, 40);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    /* unsigned int boxVAO  = generateBuffer(box); */
    /* unsigned int padVAO  = generateBuffer(pad); */

    // Construct scene
    rootNode = createSceneNode();
    /* boxNode  = createSceneNode(); */
    /* padNode  = createSceneNode(); */
    ballNode = createSceneNode();

	lightNode1 = createSceneNode();
	lightNode2 = createSceneNode();
	lightNode3 = createSceneNode();

    rootNode->children.push_back(textNode);

	newBox(rootNode, glm::vec3(1), GEOMETRY, glm::vec3(0, 0, 4));

    rootNode->children.push_back(ballNode);
	ballNode->position = glm::vec3(0, 0, 1);
	ballNode->nodeType = GEOMETRY;

    /* rootNode->children.push_back(padNode); */
    /* rootNode->children.push_back(ballNode); */

	
	lightNode1->nodeType = POINT_LIGHT;
	lightNode2->nodeType = POINT_LIGHT;
	lightNode3->nodeType = POINT_LIGHT;

	/* lightNode1->position = glm::vec3(0, 50, 0); */
	textNode->position = glm::vec3(10, windowHeight - 50, 0);

	lightNode1->position = glm::vec3(0, 30, 0);
	lightNode2->position = glm::vec3(1, 0, 0);
	lightNode3->position = glm::vec3(-1, 0, 0);
	ballNode->children.push_back(lightNode1);


	/* boxNode->children.push_back(lightNode1); */
	/* padNode->children.push_back(lightNode2); */
	/* padNode->children.push_back(lightNode3); */


	/* boxNode->nodeType = GEOMETRY; */

	/* boxNode->textureID = loadTextureFromImage(diffuseTexture); */
	/* boxNode->normalMapTextureID = loadTextureFromImage(normalMap); */
	/* boxNode->roughnessID = loadTextureFromImage(roughnessMap); */



    /* boxNode->vertexArrayObjectID = boxVAO; */
    /* boxNode->VAOIndexCount = box.indices.size(); */

    /* padNode->vertexArrayObjectID = padVAO; */
    /* padNode->VAOIndexCount = pad.indices.size(); */

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();


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
