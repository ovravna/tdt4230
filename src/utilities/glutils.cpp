#include <glad/glad.h>
#include <program.hpp>
#include "glutils.h"
#include <vector>

template <class T>
unsigned int generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize) {
    unsigned int bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
    glEnableVertexAttribArray(id);
    return bufferID;
}

void updateTextureCoordinates(unsigned int VAO, Mesh mesh) {

    glBindVertexArray(VAO);
    generateAttribute(2, 2, mesh.textureCoordinates, false);
	glBindVertexArray(0);
}

unsigned int generateBuffer(Mesh &mesh) {
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);


    generateAttribute(0, 3, mesh.vertices, false);
    generateAttribute(1, 3, mesh.normals, true);
	

    if (mesh.textureCoordinates.size() > 0) {

		for (unsigned int i=0; i < mesh.vertices.size(); i+=3){

			// Shortcuts for vertices
			glm::vec3 & v0 = mesh.vertices[i+0];
			glm::vec3 & v1 = mesh.vertices[i+1];
			glm::vec3 & v2 = mesh.vertices[i+2];

			// Shortcuts for UVs
			glm::vec2 & uv0 = mesh.textureCoordinates[i+0];
			glm::vec2 & uv1 = mesh.textureCoordinates[i+1];
			glm::vec2 & uv2 = mesh.textureCoordinates[i+2];

			// Edges of the triangle : position delta
			glm::vec3 deltaPos1 = v1-v0;
			glm::vec3 deltaPos2 = v2-v0;

			// UV delta
			glm::vec2 deltaUV1 = uv1-uv0;
			glm::vec2 deltaUV2 = uv2-uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
			//
			// Set the same tangent for all three vertices of the triangle.
			// They will be merged later, in vboindexer.cpp
			mesh.tangents.push_back(tangent);
			mesh.tangents.push_back(tangent);
			mesh.tangents.push_back(tangent);

			// Same thing for bitangents
			mesh.bitangents.push_back(bitangent);
			mesh.bitangents.push_back(bitangent);
			mesh.bitangents.push_back(bitangent);

		}

        generateAttribute(2, 2, mesh.textureCoordinates, false);
		generateAttribute(3, 3, mesh.tangents, false);
		generateAttribute(4, 3, mesh.bitangents, false);
    }

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
	
	/* GLuint tangentbuffer; */
    /* glGenBuffers(1, &tangentbuffer); */
    /* glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer); */
    /* glBufferData(GL_ARRAY_BUFFER, mesh.tangents.size() * sizeof(glm::vec3), mesh.tangents.data(), GL_STATIC_DRAW); */

    /* GLuint bitangentbuffer; */
    /* glGenBuffers(1, &bitangentbuffer); */
    /* glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer); */
    /* glBufferData(GL_ARRAY_BUFFER, mesh.bitangents.size() * sizeof(glm::vec3), mesh.bitangents.data(), GL_STATIC_DRAW); */

    return vaoID;
}
