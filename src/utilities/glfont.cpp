#include <iostream>
#include "glfont.h"

float fontW = 29, fontH = 39, fontSize = 128;

void clearText(Mesh * mesh) {
	for (unsigned int i = 0; i < mesh->textureCoordinates.size(); i++) {

		mesh->textureCoordinates.at(i) = { 0, 0 };
	}

}

void setText(Mesh * mesh, std::string text) {
	
    for(unsigned int i = 0; i < mesh->textureCoordinates.size() / 4; i++)
    {
		// 
		float u = 1.0f / 128.0f;


		int idx = 0; // = text[i];
		if (i < text.length()) {
			idx = text[i];
			mesh->textureCoordinates.at(4 * i + 0) = { idx * u, 0 };
			mesh->textureCoordinates.at(4 * i + 1) = { (idx + 1) * u , 0 }; 
			mesh->textureCoordinates.at(4 * i + 2) = { (idx + 1) * u , 1 };
			mesh->textureCoordinates.at(4 * i + 3) = { idx * u, 1 };
		}
		else  {
			mesh->textureCoordinates.at(4 * i + 0) = { 0, 0 };
			mesh->textureCoordinates.at(4 * i + 1) = { 0, 0 }; 
			mesh->textureCoordinates.at(4 * i + 2) = { 0, 0 };
			mesh->textureCoordinates.at(4 * i + 3) = { 0, 0 };
		}


   }
}

Mesh generateTextGeometryBuffer(size_t size) {
    float characterWidth = fontW;
    float characterHeight = fontH;

    unsigned int vertexCount = 4 * size;
    unsigned int indexCount = 6 * size;

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);

    mesh.textureCoordinates.resize(vertexCount);

    for(unsigned int i = 0; i < size; i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};

		// 
		/* float u = 1.0f / 128.0f; */
		/* int idx = text[i]; */
        /* mesh.textureCoordinates.at(4 * i + 0) = { idx * u, 0 }; */
        /* mesh.textureCoordinates.at(4 * i + 1) = { (idx + 1) * u , 0 }; */ 
        /* mesh.textureCoordinates.at(4 * i + 2) = { (idx + 1) * u , 1 }; */
        /* mesh.textureCoordinates.at(4 * i + 3) = { idx * u, 1 }; */

        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

    }
	/* updateText(&mesh, text); */


    return mesh;

}

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);

    mesh.textureCoordinates.resize(vertexCount);

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};

		// 
		/* float u = 1.0f / 128.0f; */
		/* int idx = text[i]; */
        /* mesh.textureCoordinates.at(4 * i + 0) = { idx * u, 0 }; */
        /* mesh.textureCoordinates.at(4 * i + 1) = { (idx + 1) * u , 0 }; */ 
        /* mesh.textureCoordinates.at(4 * i + 2) = { (idx + 1) * u , 1 }; */
        /* mesh.textureCoordinates.at(4 * i + 3) = { idx * u, 1 }; */

        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

    }
	setText(&mesh, text);


    return mesh;
}
