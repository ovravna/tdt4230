#pragma once

#include <string>
#include "mesh.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth);
Mesh generateTextGeometryBuffer(size_t size);
void setTextCoodinates(Mesh * mesh, std::string text);
void clearText(Mesh * mesh);
