#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform int drawMode; // 0=3D, 1=2D, 2=Normal

uniform layout(location = 3) mat4 model;
uniform layout(location = 4) mat4 view;
uniform layout(location = 5) mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 orthoProjection;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out vec3 fragmentPos;

void main()
{
	
	/* if (drawMode == 0) { */
		normal_out = normalize(normalMatrix * normal_in);

		/* normal_out = mat3(transpose(inverse(model))) * normal_in; */

		/* normal_out = normalize(mat3(model) * normal_in); */ 

		textureCoordinates_out = textureCoordinates_in;
		gl_Position = projection * view * model * vec4(position, 1.0f);

		fragmentPos = vec3(model * vec4(position, 1.0));
	/* } */
	/* else if (drawMode == 1) { */

	/* 	/1* gl_Position = orthoProjection * vec4(position, 1.0f); *1/ */
	/* 	gl_Position = orthoProjection * vec4(position, 1.0f); */
	/* 	textureCoordinates_out = textureCoordinates_in; */
	/* } */
}
