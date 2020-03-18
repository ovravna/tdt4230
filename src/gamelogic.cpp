
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

SceneNode * portal0, * portal1;
glm::mat4 portalViewMatrix;

SceneNode * outlinedNodes[2];

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

SceneNode* rootNode;

SceneNode* textNode;

double timeDelta;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shaderText;
Gloom::Shader* shaderSingleColor;
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


SceneNode * newBox(
		SceneNode * parent,
		glm::vec3 dimentions,
		glm::vec3 position = glm::vec3(0),
		Mesh * mesh = nullptr,
		glm::vec4 color = glm::vec4(1),
		SceneNodeType nodeType = GEOMETRY,
		glm::vec3 rotation = glm::vec3(0),
		glm::vec3 referencePoint = glm::vec3(0)
		)  {

	SceneNode * node = createSceneNode();
	node->nodeType = nodeType;
	node->position = position;
	node->referencePoint = referencePoint;
	node->rotation = rotation;
	node->currentTransformationMatrix = glm::mat4(1);
	node->color = color;
	
	Mesh box;
	if (mesh == nullptr) 
		box = cube(dimentions);
	else 
		box = *mesh;

    node->vertexArrayObjectID = generateBuffer(box);
	node->VAOIndexCount = box.indices.size();

	parent->children.push_back(node);

	return node;

}

SceneNode * newBox(SceneNode * parent, glm::vec3 position, glm::vec4 color) {
	return newBox(parent, glm::vec3(1), position, nullptr, color);    
}

SceneNode * initText(SceneNode * parent) {

	PNGImage charmap = loadPNGFile("res/textures/charmap.png");
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

SceneNode * newLight(SceneNode * parent, glm::vec3 position, glm::vec4 color = glm::vec4(1)) {

	SceneNode * lightNode = createSceneNode();
	lightNode->nodeType = POINT_LIGHT;
	lightNode->position = position;
	lightNode->color = color;
	addChild(parent, lightNode);
	return lightNode;
}

SceneNode * newPortal(SceneNode * parent, glm::vec3 position, glm::vec4 color, glm::vec3 rotation) {

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
	cam->position = glm::vec3(1, 0.5, 0);
	cam->front = glm::vec3(1, 0, -1);

    shader = new Gloom::Shader();
    shader->makeBasicShader("res/shaders/simple.vert", "res/shaders/simple.frag");

    shaderText = new Gloom::Shader();
    shaderText->makeBasicShader("res/shaders/text.vert", "res/shaders/text.frag");

    shaderSingleColor = new Gloom::Shader();
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


	/* PNGImage diffuseTexture = loadPNGFile("res/textures/Brick03_col.png"); */
	/* PNGImage normalMap = loadPNGFile("res/textures/Brick03_nrm.png"); */
	/* PNGImage roughnessMap = loadPNGFile("res/textures/Brick03_rgh.png"); */
	


    // Construct scene
    rootNode = createSceneNode();

	initText(rootNode);
    /* newBall(rootNode, 1.0, 40, 40, glm::vec3(0, 0, 1)); */
	/* newBox(rootNode, glm::vec3(1), glm::vec3(0, 0, 4)); */

	int size = 10;
	auto box = cube(glm::vec3(1));
	/* for (int y = 0; y < size; y++) */ 
	/* 	for (int x = 0; x < size; x++) { */
	/* 		float a = rand(x, y); */
	/* 		float b = rand(2 * x, 2 * y); */
	/* 		float c = rand(3 * x, 3 * y); */
	/* 		newBox(rootNode, glm::vec3(1), glm::vec3(x, 0, y), &box, glm::vec4(a, b, c, 1), GEOMETRY); */
	/* 		newBox(rootNode, glm::vec3(1), glm::vec3(size, x, y), &box, glm::vec4(a, b, c, 1), GEOMETRY); */
	/* 		newBox(rootNode, glm::vec3(1), glm::vec3(0, x, y), &box, glm::vec4(a, b, c, 1), GEOMETRY); */

	/* 	} */

	auto o0 = newBox(rootNode, glm::vec3(1), glm::vec3(3, 0, 3), &box, glm::vec4(1, 0, 0, 1), GEOMETRY);  
	auto o1 = newBox(rootNode, glm::vec3(1), glm::vec3(2, 0, 6), &box, glm::vec4(0.2, 0.4, 1, 1), GEOMETRY);  

	/* o0->textureID = loadTextureFromImage(diffuseTexture); */
	/* o0->normalMapTextureID = loadTextureFromImage(normalMap); */
	/* o0->roughnessID = loadTextureFromImage(roughnessMap); */

	/* o1->textureID = loadTextureFromImage(diffuseTexture); */
	/* o1->normalMapTextureID = loadTextureFromImage(normalMap); */
	/* o1->roughnessID = loadTextureFromImage(roughnessMap); */

	outlinedNodes[0] = o0;
	outlinedNodes[1] = o1;
	
	auto p = plane();
	portal0 =  newBox(rootNode, glm::vec3(1), glm::vec3(0, 0, -2), &p, glm::vec4(0, 1, 0, 1), GEOMETRY_PORTAL, glm::vec3(0, M_PI_2, 0)); 
	portal1 =  newBox(rootNode, glm::vec3(1), glm::vec3(2, 0, -4), &p, glm::vec4(0, 1, 0, 1), GEOMETRY_PORTAL, glm::vec3(0, 0, 0)); 

	/* Box::create(rootNode) */
	/* 	->rotate(glm::vec3(0, M_PI_4, 0)) */
	/* 	->move(glm::vec3(1.f, 0.f, -4.f)) */
	/* 	->generateMesh() */
	/* 	->generateVAO(); */
		

	
	newLight(rootNode, glm::vec3(7, 10, 3), glm::vec4(1, 1, 1, 1));
	/* newLight(rootNode, glm::vec3(9, 13, 9), glm::vec4(0, 1, 0, 1)); */
	/* newLight(rootNode, glm::vec3(1, 8, 12), glm::vec4(0, 0, 1, 1)); */

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

			/* node->color = glm::vec4(glm::normalize(cam->front), 1.0f); */

			glUniform4fv(lightIdx + 6, 1, glm::value_ptr(node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1)));
    		glUniform4fv(lightIdx + 9, 1, glm::value_ptr(node->color));
			
			/* node->color *= glm::rotate(sinf(lightFloat), glm::vec3(1) ); */

			/* node->color.r = sinf(lightFloat) / 2 + 0.5f; */
			/* node->color.g = cosf(lightFloat) / 2 + 0.5f; */
		
			/* node->color.b = sinf(lightFloat) / 2 + 0.5f; */
			/* if (node->color[lightIdx] >= 1.0) */
				/* node->color[lightIdx] = 0.0; */


			lightIdx++;

			break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void getOutlined(SceneNode * node, std::vector<SceneNode*> * accumulator) {
	if (node->nodeType == GEOMETRY_STENCIL_OUTLINED)
		accumulator->push_back(node);

	
    for(SceneNode* child : node->children) {
        getOutlined(child, accumulator);
    }


}

void renderNodeOutlined(SceneNode * node) {
	std::vector<SceneNode*> * accumulator = new std::vector<SceneNode*>();
	getOutlined(node, accumulator);

	glad_glUniform1i(drawModeLoc, 0);

	for (SceneNode * node : *accumulator) {

		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer
		shader->activate();
		
		glUniform4fv(12, 1, glm::value_ptr(node->color));
		glBindVertexArray(node->vertexArrayObjectID);
		glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

	}
	for (SceneNode * node : *accumulator) {
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00); // disable writing to the stencil buffer
		glDisable(GL_DEPTH_TEST);
		shaderSingleColor->activate();

		/* node->currentTransformationMatrix *= glm::scale(glm::vec3(1.5)); */
		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(glm::scale(node->currentTransformationMatrix, glm::vec3(1.2))));
		/* glUniform4fv(12, 1, glm::value_ptr(node->color)); */
		glBindVertexArray(node->vertexArrayObjectID);
		glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

		//clean up 
		shader->activate();
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);   
		glEnable(GL_DEPTH_TEST);  

	}

}

void renderOutlined(SceneNode * node ) {
	glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

	auto nm = glm::mat3(glm::transpose(glm::inverse(node->currentTransformationMatrix)));
	auto mv3x3 = glm::mat3(cam->getView() * node->currentTransformationMatrix);

    glUniformMatrix3fv(normalMatricLoc, 1, GL_FALSE, glm::value_ptr(nm));
    glUniformMatrix3fv(mv3x3Loc, 1, GL_FALSE, glm::value_ptr(mv3x3));


	switch (node->nodeType) {

		

		case GEOMETRY_STENCIL_OUTLINED:
			if (node->vertexArrayObjectID != -1) {

				glad_glUniform1i(drawModeLoc, 0);
				/* glEnable(GL_DEPTH_TEST); */
				/* glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); */  
				  
				/* glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); */ 

				/* glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix)); */

				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
				glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
				glStencilMask(0xFF); // enable writing to the stencil buffer
				shader->activate();
				
				glUniform4fv(12, 1, glm::value_ptr(node->color));
				glBindVertexArray(node->vertexArrayObjectID);
				glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);


				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00); // disable writing to the stencil buffer
				glDisable(GL_DEPTH_TEST);
				shaderSingleColor->activate();

				node->currentTransformationMatrix *= glm::scale(glm::vec3(1.5));
				glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
				/* glUniform4fv(12, 1, glm::value_ptr(node->color)); */
				glBindVertexArray(node->vertexArrayObjectID);
				glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

				//clean up 
				shader->activate();
				glStencilMask(0xFF);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);   
				glEnable(GL_DEPTH_TEST);  

				 
			}
			break;
		default:
			break;
	}

    for(SceneNode* child : node->children) {
        renderOutlined(child);
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

void renderOut() {

		/* shaderSingleColor.setMat4("view", view); */
        /* shaderSingleColor.setMat4("projection", projection); */

        shader->activate();

		glUniformMatrix4fv(4, 1, GL_FALSE, cam->getViewPtr());
		glUniformMatrix4fv(5, 1, GL_FALSE, cam->getProjectionPtr());

        // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
        glStencilMask(0x00);
        // floor
        /* glBindVertexArray(planeVAO); */
        /* glBindTexture(GL_TEXTURE_2D, floorTexture); */
        /* shader.setMat4("model", glm::mat4(1.0f)); */
        /* glDrawArrays(GL_TRIANGLES, 0, 6); */
        /* glBindVertexArray(0); */

        // 1st. render pass, draw objects as normal, writing to the stencil buffer
        // --------------------------------------------------------------------
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
		auto cube = outlinedNodes[0];
        // cubes
        glBindVertexArray(cube->vertexArrayObjectID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube->textureID);
        /* model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f)); */

		glUniform4fv(12, 1, glm::value_ptr(cube->color));
        /* shader.setMat4("model", model); */
		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(cube->currentTransformationMatrix));
        /* glDrawArrays(GL_TRIANGLES, 0, 36); */
		glDrawElements(GL_TRIANGLES, cube->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

		cube = outlinedNodes[1];
		glUniform4fv(12, 1, glm::value_ptr(cube->color));
        /* model = glm::mat4(1.0f); */
        /* model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f)); */
        /* shader.setMat4("model", model); */
		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(cube->currentTransformationMatrix));
        /* glDrawArrays(GL_TRIANGLES, 0, 36); */
		glDrawElements(GL_TRIANGLES, cube->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

        // 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
        // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
        // the objects' size differences, making it look like borders.
        // -----------------------------------------------------------------------------------------------------------------------------
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        shaderSingleColor->activate();
        float scale = 1.1;
        // cubes
		cube = outlinedNodes[0];
        glBindVertexArray(cube->vertexArrayObjectID);
        glBindTexture(GL_TEXTURE_2D, cube->textureID);
        /* model = glm::mat4(1.0f); */
        /* model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f)); */
        /* model = glm::scale(model, glm::vec3(scale, scale, scale)); */
        /* shaderSingleColor.setMat4("model", model); */
		/* cube->currentTransformationMatrix *= glm::scale(glm::vec3(scale)); */
		cube->currentTransformationMatrix = glm::scale(cube->currentTransformationMatrix, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(cube->currentTransformationMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
		/* glDrawElements(GL_TRIANGLES, cube->VAOIndexCount, GL_UNSIGNED_INT, nullptr); */
        /* model = glm::mat4(1.0f); */
        /* model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f)); */
        /* model = glm::scale(model, glm::vec3(scale, scale, scale)); */
        /* shaderSingleColor.setMat4("model", model); */
		cube->currentTransformationMatrix = glm::scale(cube->currentTransformationMatrix, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(cube->currentTransformationMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
		/* glDrawElements(GL_TRIANGLES, cube->VAOIndexCount, GL_UNSIGNED_INT, nullptr); */
        glBindVertexArray(0);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);


}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);


    glUniform3fv(camLoc, 1, glm::value_ptr(cam->position));
    glUniformMatrix4fv(4, 1, GL_FALSE, cam->getViewPtr());
    glUniformMatrix4fv(5, 1, GL_FALSE, cam->getProjectionPtr());

	/* renderOut(); */



    /* glUniform4fv(6, 1, glm::value_ptr(lights[0])); */
    /* glUniform4fv(7, 1, glm::value_ptr(lights[1])); */
    /* glUniform4fv(8, 1, glm::value_ptr(lights[2])); */

	lightIdx = 0;
	lightFloat += 0.01;
    renderNode(rootNode);
	/* renderNodeOutlined(rootNode); */ 



	renderText(textNode);


}


void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {

		auto pos = cam->position;
		auto front = cam->front;
		auto next = glm::floor(pos) + glm::round(2.0f * glm::normalize(front));
		
		newBox(rootNode, glm::vec3(1), glm::vec3(next.x, 1, next.z), nullptr, glm::vec4(0.2, 0.3, 1, 1));
	}

	cam->handleKeyboardInput();
}
