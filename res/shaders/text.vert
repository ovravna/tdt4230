#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
out layout(location = 1) vec2 textureCoordinates;

uniform layout(location = 3) mat4 model;
uniform layout(location = 4) mat4 view;
uniform layout(location = 5) mat4 projection;

uniform mat4 orthoProjection;
uniform vec3 textPos;

void main() {

		textureCoordinates = textureCoordinates_in;
		gl_Position =  orthoProjection * view * model * vec4(position + textPos, 1.0f);

}


