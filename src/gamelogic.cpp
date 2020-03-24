
#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <shader.hpp>
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
#include "entity.hpp"

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "camera.hpp"
#include "utilities/bodies.hpp"

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

Mesh portals[2];

glm::vec4 lights[3];
glm::vec3 lightColors[3] {
	glm::vec3(1, 0, 0),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 0, 1),

};

Entity * portal0, * portal1;
Cube * awesomeCube;
glm::mat4 portalViewMatrix;


int lightIdx = 0;
float lightFloat = 0.0f;
GLint normalMatricLoc;
GLint mv3x3Loc;
GLint camLoc;
GLint portalLoc;
GLint ballLoc;
GLint lightSourcesLoc;
GLint drawModeLoc;

GLint orthoProjectionLoc;
GLint textPosLoc;

Camera * cam;
Mesh text;

RootEntity* rootNode;

Text* textNode;

double timeDelta;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Shader* shader;
Shader* shaderText;
Shader* shaderSingleColor;
sf::Sound* sound;

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


/**
 * Compute a world2camera view matrix to see from portal 'dst', given
 * the original view and the 'src' portal position.
 */
glm::mat4 portalView(glm::mat4 origView, SceneNode* src, SceneNode* dst) {
  glm::mat4 mv = origView * src->currentTransformationMatrix;
  glm::mat4 portalCam =
    // 3. transformation from source portal to the camera - it's the
    //    first portal's ModelView matrix:
    mv
    // 2. object is front-facing, the camera is facing the other way:
    * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0.0,1.0,0.0))
    // 1. go the destination portal; using inverse, because camera
    //    transformations are reversed compared to object
    //    transformations:
    * glm::inverse(dst->currentTransformationMatrix)
    ;
  return portalCam;
}
//// A few lines to help you if you've never used c++ structs


float fract (float value) { return value - std::floor(value); }
float rand(float x, float y) { return fract(sinf(x * 12.9898 + y * 78.233) * 43758.5453); }

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("res/Hall of the Mountain King.ogg")) {
        return;
    }
    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

	cam = new Camera(window);
	cam->position = glm::vec3(1, 1.7, 0);
	cam->front = glm::vec3(1, 0, -1);

    shader = new Shader();
    shader->makeBasicShader("res/shaders/simple.vert", "res/shaders/simple.frag");

    shaderText = new Shader();
    shaderText->makeBasicShader("res/shaders/text.vert", "res/shaders/text.frag");

    shaderSingleColor = new Shader();
    shaderSingleColor->makeBasicShader("res/shaders/simple.vert", "res/shaders/singleColor.frag");

    shader->activate();

	normalMatricLoc = glad_glGetUniformLocation(shader->get(), "normalMatrix");
	mv3x3Loc = glad_glGetUniformLocation(shader->get(), "MV3x3");
	camLoc = glad_glGetUniformLocation(shader->get(), "camPos");
	portalLoc = glad_glGetUniformLocation(shader->get(), "portalView");
	ballLoc = glad_glGetUniformLocation(shader->get(), "ballPos");
	lightSourcesLoc = glad_glGetUniformLocation(shader->get(), "lightSources");
	drawModeLoc = glad_glGetUniformLocation(shader->get(), "drawMode");

	orthoProjectionLoc = glad_glGetUniformLocation(shaderText->get(), "orthoProjection");
	textPosLoc = glad_glGetUniformLocation(shaderText->get(), "textPos");




    // Construct scene
    rootNode = RootEntity::create().get();

	textNode = (Text*) Text::create(30)
		.place(10, windowHeight - 50, 0)
		.get();

	int size = 10;

	for (int y = 0; y < size; y++) 
		for (int x = 0; x < size; x++) {
			float a = rand(x, y);
			float b = rand(2 * x, 2 * y);
			float c = rand(3 * x, 3 * y);

			auto cube = Cube::create()
				.setParent(rootNode)
				.place(x, 0, y)
				.setColor(a, b, c);

			Cube::create(cube.mesh)
				.setParent(rootNode)
				.place(size, x, y)
				.setColor(a, b, c);

			Cube::create(cube.mesh)
				.setParent(rootNode)
				.place(0, x, y)
				.setColor(a, b, c);
		}


	auto o = Cube::create()
		.setParent(rootNode)
		.place(3, 1, 3)
		.setColor(1, 0, 0);
		/* .setType(GEOMETRY_NORMAL_MAPPED) */
		/* .setTexure("res/textures/Brick03_col.png", DIFFUSE) */
		/* .setTexure("res/textures/Brick03_nrm.png", NORMAL) */
		/* .setTexure("res/textures/Brick03_rgh.png", ROUGHNESS); */

	Cube::create(o.mesh)
		.setParent(rootNode)
		.place(2, 1, 6)
		.setColor(0, 0, 1);
		/* .setType(GEOMETRY_NORMAL_MAPPED) */
		/* .setTexure(o.textureID, DIFFUSE) */
		/* .setTexure(o.normalMapTextureID, NORMAL) */
		/* .setTexure(o.roughnessID, ROUGHNESS); */

	portal0 = Portal::create()
		.setParent(rootNode)
		.place(0, 1, -2)
		.setColor(0, 1, 0)
		.setType(GEOMETRY_PORTAL)
		.rotate(0, M_PI_2, 0)
		.get();

	portal1 = Portal::create(portal0->mesh)
		.setParent(rootNode)
		.place(2, 1, -4)
		.setColor(0, 1, 0)
		.setType(GEOMETRY_PORTAL)
		.get();

	awesomeCube = Cube::create()
		.setParent(rootNode)
		.setType(GEOMETRY)
		.setColor(1, 1, 0)
		.place(1, 1, -2)
		.setReferencePoint(0.5, 0, 0)
		.rotate(0, M_PI_4, 0)
		.as<Cube>().get();
	
	Light::create()
		.setParent(rootNode)
		.place(7, 10, 3)
		.setColor(1, 1, 1);

    /* glUniform3fv(9, 1, glm::value_ptr(lightColors[0])); */
    /* glUniform3fv(10, 1, glm::value_ptr(lightColors[1])); */
    /* glUniform3fv(11, 1, glm::value_ptr(lightColors[2])); */

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

	portalViewMatrix = portalView(cam->getView(), portal0, portal1); 

    glUniformMatrix3fv(normalMatricLoc, 1, GL_FALSE, glm::value_ptr(nm));
    glUniformMatrix3fv(mv3x3Loc, 1, GL_FALSE, glm::value_ptr(mv3x3));


    switch(node->nodeType) {
        case GEOMETRY:
        case GEOMETRY_PORTAL:
            if(node->vertexArrayObjectID != -1) {

				if (node->nodeType == GEOMETRY_PORTAL) 
					glad_glUniform1i(drawModeLoc, 3);
				else 
					glad_glUniform1i(drawModeLoc, 0);
				glUniformMatrix4fv(portalLoc, 1, GL_FALSE, glm::value_ptr(portalViewMatrix));

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
		case GEOMETRY_STENCIL_OUTLINED:
			
			break;
		case GEOMETRY_2D:
			break;
        case POINT_LIGHT: 

			glUniform4fv(lightIdx + 6, 1, glm::value_ptr(node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1)));
    		glUniform4fv(lightIdx + 9, 1, glm::value_ptr(node->color));
	
			lightIdx++;

			break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderText(Text * node) {
		shaderText->activate();
		glBindTextureUnit(0, node->textureID);

		int fps = std::round(1.0f / timeDelta);
		node->setText( fmt::format("{}", fps) );
		/* setText(&text, glm::to_string(cam->front) ); */

		/* updateTextureCoordinates(node->vertexArrayObjectID, text); */

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


    glUniform3fv(camLoc, 1, glm::value_ptr(cam->position));
    glUniformMatrix4fv(4, 1, GL_FALSE, cam->getViewPtr());
    glUniformMatrix4fv(5, 1, GL_FALSE, cam->getProjectionPtr());

	awesomeCube->rotate(-0.005, 0.01, 0);

	lightIdx = 0;
	lightFloat += 0.01;

    renderNode(rootNode);
	rootNode->drawAllGeometry();

	renderText(textNode);
}


void handleKeyboardInput(GLFWwindow* window) {
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {

		auto pos = cam->position;
		auto front = cam->front;
		auto next = glm::floor(pos) + glm::round(2.0f * glm::normalize(front));
		
		Cube::create()
			.setParent(rootNode)
			.place(next.x, 1, next.z)
			.setColor(0.2, 0.3, 1);
	}

	cam->handleKeyboardInput();
}
