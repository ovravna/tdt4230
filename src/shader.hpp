#ifndef MY_SHADER_HPP
#define MY_SHADER_HPP


#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <utilities/shader.hpp>


enum TextureType {
	DIFFUSE, NORMAL, ROUGHNESS
};

class Shader : public Gloom::Shader {

public:
	
	Shader() : Gloom::Shader{} {}

	int getLocation(std::string name) { return glad_glGetUniformLocation(this->get(), name.c_str()); }


	void setMat(std::string location, glm::mat4 value, int size = 1, bool transpose = false) {
		auto loc = getLocation(location);
    	glUniformMatrix4fv(loc, size, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(value));
	}

	void setMat(std::string location, glm::mat3 value, int size = 1, bool transpose = false) {
		auto loc = getLocation(location);
    	glUniformMatrix3fv(loc, size, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(value));
	}

	void setVec(std::string location, glm::vec3 value, int size = 1) {
		auto loc = getLocation(location);
    	glUniform3fv(loc, size, glm::value_ptr(value));
	}

	void setVec(std::string location, glm::vec4 value, int size = 1) {
		auto loc = getLocation(location);
    	glUniform4fv(loc, size, glm::value_ptr(value));
	}
	
};

#endif
