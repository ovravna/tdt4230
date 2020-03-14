#ifndef GAME_TOOLS_H
#define GAME_TOOLS_H

#include "glad/glad.h"
#include "glm/fwd.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sceneGraph.hpp"
#include "utilities/imageLoader.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

GLint loadTextureFromImage(PNGImage img);
void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);



#endif
