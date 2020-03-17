
#include "glm/ext/vector_float3.hpp"
#include "sceneGraph.hpp"
#include "utilities/glutils.h"
#include "utilities/mesh.h"
#include "utilities/shapes.h"

struct SceneNodeData {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec4 color;

	glm::mat4 currentTransformationMatrix;

	glm::vec3 referencePoint;

	SceneNodeType nodeType;

};



template <class T>
class Body {
	protected:

		SceneNode * node;
		Mesh mesh;

		/* glm::vec3 position; */
		/* glm::vec3 dimentions; */
		/* glm::vec3 rotation; */
		/* glm::vec3 referencePoint; */

	private:
		void init(SceneNode * parent) {
			node = createSceneNode();
			node->currentTransformationMatrix = glm::mat4(1);
			addChild(parent, node);
		}

	public:

		class create;

		/* Body(SceneNode * parent) { */
		/* 	init(parent); */
		/* } */

		~Body() {
			delete node;
		}

		Body(SceneNode * parent,
				glm::vec3 position = glm::vec3(0), 
				glm::vec4 color = glm::vec4(1), 
				SceneNodeType nodeType = GEOMETRY, 
				glm::vec3 referencePoint = glm::vec3(0), 
				glm::vec3 rotation = glm::vec3(0)
			){ 

			init(parent);
			setNodeValues(position, color, nodeType, referencePoint, rotation);
		}


		/* static Body<T> create(SceneNode * parent) { */
		/* 	return new Body<T>(parent); */
		/* } */

		Body<T> * rotate(glm::vec3 rotation) {
			node->rotation = rotation;
			return this; 
		}

		Body<T> * move(glm::vec3 position) {
			node->position = position;
			return this; 
		}
		
		Body<T> * generateMesh(T data) {
			createMesh(data);
			return this; 
		}

		Body<T> * generateVAO() {
			generateVertexArrayObject(mesh);
			return this; 
		}


		


		
		Body<T> * setNodeValues(
				glm::vec3 position = glm::vec3(0), 
				glm::vec4 color = glm::vec4(1), 
				SceneNodeType nodeType = GEOMETRY, 
				glm::vec3 referencePoint = glm::vec3(0), 
				glm::vec3 rotation = glm::vec3(0)
				)  {

			node->nodeType = nodeType;
			node->position = position;
			node->referencePoint = referencePoint;
			node->rotation = rotation;
			node->color = color;

			return this;

		}


		void generateVertexArrayObject(Mesh mesh) {
			this->mesh = mesh;
			node->vertexArrayObjectID = generateBuffer(mesh);
			node->VAOIndexCount = mesh.indices.size();
		}

		glm::vec3 getPosition() { return node->position; }
		Mesh getMesh() { return mesh; }

		
		virtual Mesh createMesh(T data);
		void generateMeshVertexArrayObject(T data) {
			createMesh(data);
			generateVertexArrayObject(mesh);
		}

};

template <class T>
class Body<T>::create {

	public: 
		create() {

		}

		Body<T> setParent(SceneNode * parent) {
			this->parent = parent;
		}
		Body<T> build() {
			return new Body<T>(parent);

		}

	private:
		SceneNode * parent;


}; 
class Box : public Body<glm::vec3> {
	public:
		Box(SceneNode * parent,
				glm::vec3 position = glm::vec3(0), 
				glm::vec4 color = glm::vec4(1), 
				SceneNodeType nodeType = GEOMETRY, 
				glm::vec3 referencePoint = glm::vec3(0), 
				glm::vec3 rotation = glm::vec3(0)
			) : Body{parent, position, color, nodeType, referencePoint, rotation} { }


		Mesh createMesh(glm::vec3 data) {
			mesh = cube( data );
			return mesh;
		}
};

/* class Sphere : public Body<float*> { */
/* 	Mesh createMesh(float * data) { */
/* 		mesh = generateSphere( data[0], data[1], data[2] ); */
/* 		return mesh; */
/* 	} */
/* }; */
