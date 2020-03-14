
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

	std::string t = "Ninja";

	text = generateTextGeometryBuffer(30); 
	/* updateText(&text, t); */



	textNode = createSceneNode();
	textNode->vertexArrayObjectID = generateBuffer(text);
	textNode->nodeType = GEOMETRY_2D;
	textNode->textureID = loadTextureFromImage(charmap);
	textNode->VAOIndexCount = text.indices.size();
	textNode->position = glm::vec3(0);



	normalMatricLoc = glad_glGetUniformLocation(shader->get(), "normalMatrix");
	normalMatricLoc = glad_glGetUniformLocation(shader->get(), "MV3x3");
	camLoc = glad_glGetUniformLocation(shader->get(), "camPos");
	ballLoc = glad_glGetUniformLocation(shader->get(), "ballPos");
	lightSourcesLoc = glad_glGetUniformLocation(shader->get(), "lightSources");
	drawModeLoc = glad_glGetUniformLocation(shader->get(), "drawMode");

	orthoProjectionLoc = glad_glGetUniformLocation(shaderText->get(), "orthoProjection");
	textPosLoc = glad_glGetUniformLocation(shaderText->get(), "textPos");


	orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));
    projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, false);
    Mesh sphere = generateSphere(1.0, 40, 40);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);

    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    ballNode = createSceneNode();

	lightNode1 = createSceneNode();
	lightNode2 = createSceneNode();
	lightNode3 = createSceneNode();

    rootNode->children.push_back(textNode);

    rootNode->children.push_back(ballNode);
	ballNode->position = glm::vec3(0, 0, 3);

    /* rootNode->children.push_back(padNode); */
    /* rootNode->children.push_back(ballNode); */

	
	lightNode1->nodeType = POINT_LIGHT;
	lightNode2->nodeType = POINT_LIGHT;
	lightNode3->nodeType = POINT_LIGHT;

	/* lightNode1->position = glm::vec3(0, 50, 0); */
	textNode->position = glm::vec3(10, windowHeight - 50, 0);

	/* lightNode1->position = glm::vec3(0, 70, 0); */
	lightNode2->position = glm::vec3(1, 0, 0);
	lightNode3->position = glm::vec3(-1, 0, 0);


	/* boxNode->children.push_back(lightNode1); */
	/* padNode->children.push_back(lightNode2); */
	/* padNode->children.push_back(lightNode3); */


	boxNode->nodeType = GEOMETRY;

	/* boxNode->textureID = loadTextureFromImage(diffuseTexture); */
	/* boxNode->normalMapTextureID = loadTextureFromImage(normalMap); */
	/* boxNode->roughnessID = loadTextureFromImage(roughnessMap); */



    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();

    padNode->vertexArrayObjectID = padVAO;
    padNode->VAOIndexCount = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();


    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}


void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }
    
    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }



    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform = 
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0));
                    /* glm::translate(-cameraPosition); */

	
	/* view = glm::lookAt(glm::vec3(0), cameraPosition + glm::vec3(0.3f + 0.2f * float(-padPositionZ*padPositionZ), lookRotation, 0), glm::vec3(0, 1, 0)); */ 

    view = cameraTransform;
	glUniform3fv(camLoc, 1, glm::value_ptr(cam->position)); 

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = { 
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x), 
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2), 
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };

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
