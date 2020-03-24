
#include "gametools.hpp"
#include "glad/glad.h"
#include "sceneGraph.hpp"
#include "utilities/glfont.h"
#include "utilities/glutils.h"
#include "utilities/imageLoader.hpp"
#include "utilities/mesh.h"
#include "utilities/shapes.h"





class Entity : public SceneNode {

protected:
	Entity(Mesh mesh) : SceneNode{} {
		setMesh(mesh);
	}

	Entity() : SceneNode{} {
	}

public:
	Mesh mesh;
	Entity * parent; 

	static Entity& create() {
		Entity * e = new Entity();
		/* addChild(parent, e); */
		return *e;
	}

	Entity& setMesh(Mesh mesh) {
		this->mesh = mesh;
		vertexArrayObjectID = generateBuffer(mesh);
		VAOIndexCount = mesh.indices.size();
		return *this;
	}

	Entity& setTexure(PNGImage img) {
		textureID = loadTextureFromImage(img);
		return *this;
	}

	Entity& setParent(Entity * node) {
		parent = node;
		addChild(node, this);
		return *this;
	}


	Entity& setType(SceneNodeType nodeType) {
		this->nodeType = nodeType;
		return *this;
	}

	Entity& setColor(glm::vec4 color) {
		this->color = color;
		return *this;
	}

	Entity& setColor(float r, float g, float b, float a = 1.0f) {
		return setColor(glm::vec4(r, g, b, a));
	}

	Entity& place(glm::vec3 position) {
		this->position = position;
		return *this;
	}

	Entity& place(float x, float y, float z) {
		return place(glm::vec3(x, y, z));
	}

	Entity& move(glm::vec3 position) {
		this->position += position;
		return *this;
	}

	Entity& move(float x, float y, float z) {
		return move(glm::vec3(x, y, z));
	}


	Entity& setReferencePoint(glm::vec3 referencePoint) {
		this->referencePoint = referencePoint;
		return *this;
	}

	Entity& setReferencePoint(float x, float y, float z) {
		return setReferencePoint(glm::vec3(x, y, z));
	}

	Entity& setRotation(glm::vec3 rotation) {
		this->rotation = rotation;
		return *this;
	}

	Entity& setRotation(float x, float y, float z) {
		return setRotation(glm::vec3(x, y, z));
	}

	Entity& rotate(glm::vec3 rotation) {
		this->rotation += rotation;
		return *this;
	}

	Entity& rotate(float x, float y, float z) {
		return rotate(glm::vec3(x, y, z));
	}


	Entity * get() {
		return this;
	}

	glm::mat4 getModel() { return currentTransformationMatrix; }
	float * getModelPtr() { return glm::value_ptr(currentTransformationMatrix); }

	bool drawable() { return vertexArrayObjectID != -1; }

	void draw() {
		glBindVertexArray(vertexArrayObjectID);
		glDrawElements(GL_TRIANGLES, VAOIndexCount, GL_UNSIGNED_INT, nullptr);
	}
};
class Text : public Entity {

protected:
	PNGImage charmap;
	std::string text = "";

	Text(Mesh mesh) : Entity{ mesh } {
		//todo: maybe make font texture loading dynamic
		charmap = loadPNGFile("res/textures/charmap.png");
		setTexure(charmap);
		setType(GEOMETRY_2D);
	}
	
public:

	static Entity& create(Mesh mesh) {
		auto e = new Text(mesh);
		return *e;
	}
	static Entity& create(size_t size = 30) {
		auto mesh = generateTextGeometryBuffer(size);
		return Text::create(mesh);
	}

	void setText(std::string text) {
		this->text = text;

		setTextCoodinates(&mesh, text);	
		updateTextureCoordinates(vertexArrayObjectID, mesh);
		return;
		for(unsigned int i = 0; i < mesh.textureCoordinates.size() / 4; i++)
		{
			// 
			float u = 1.0f / 128.0f;

			int idx = 0; // = text[i];
			if (i < text.length()) {
				idx = text[i];
				mesh.textureCoordinates.at(4 * i + 0) = { idx * u, 0 };
				mesh.textureCoordinates.at(4 * i + 1) = { (idx + 1) * u , 0 }; 
				mesh.textureCoordinates.at(4 * i + 2) = { (idx + 1) * u , 1 };
				mesh.textureCoordinates.at(4 * i + 3) = { idx * u, 1 };
			}
			else  {
				mesh.textureCoordinates.at(4 * i + 0) = { 0, 0 };
				mesh.textureCoordinates.at(4 * i + 1) = { 0, 0 }; 
				mesh.textureCoordinates.at(4 * i + 2) = { 0, 0 };
				mesh.textureCoordinates.at(4 * i + 3) = { 0, 0 };
			}
	   	}
	}
};

class Cube : public Entity {

protected:
	Cube(Mesh mesh) : Entity{ mesh} {}
	
public:
	static Entity& create(Mesh mesh) {
		auto e = new Cube(mesh);
		return *e;
	}
	static Entity& create(glm::vec3 dimentions) {
		auto mesh = cube(dimentions);
		return Cube::create(mesh);
	}
	static Entity& create(float x, float y, float z) {
		return Cube::create(glm::vec3(x, y, z));
	}
	static Entity& create(float dimentions = 1.0f) {
		return Cube::create(glm::vec3(dimentions));
	}
};

class Portal : public Entity {

protected:
	Portal(Mesh mesh) : Entity{ mesh} {}
	
public:
	static Entity& create(Mesh mesh) {
		auto e = new Portal(mesh);
		return *e;
	}
	static Entity& create() {
		auto mesh = plane();
		return Portal::create(mesh);
	}
};




class Sphere : public Entity {

protected:
	Sphere(Mesh mesh) : Entity{ mesh } {}
	
public:
	static Entity& create(Mesh mesh) {
		auto e = new Sphere(mesh);
		return *e;
	}
	static Entity& create(float radius, int slices = 40, int layers = 40) {
		auto mesh = generateSphere(radius, slices, layers);
		return Sphere::create(mesh);
	}
};
