#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;

uniform int drawMode; // 0=3D, 1=2D, 2=Normal

uniform layout(location = 3) mat4 model;
uniform layout(location = 4) mat4 view;
uniform layout(location = 5) mat4 projection;
uniform mat3 normalMatrix;
uniform mat3 MV3x3;
uniform mat4 orthoProjection;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out vec3 fragmentPos;
out mat3x3 TNB;

void main()
{

	/* mat3x3 MV3x3 = mat3(view * model); */

	vec3 normal_cs =  normalize(normal_in); 
	vec3 tangent_cs = normalize(tangent_in); 
	vec3 bitangent_cs =  normalize(bitangent_in); 

	TNB = mat3(
		tangent_cs,
		bitangent_cs,
		normal_cs
	);

	
	normal_out = normalize(normalMatrix * normal_in);

	/* normal_out = mat3(transpose(inverse(model))) * normal_in; */

	/* normal_out = normalize(mat3(model) * normal_in); */ 

	textureCoordinates_out = textureCoordinates_in;
	gl_Position = projection * view * model * vec4(position, 1.0f);

	fragmentPos = vec3(model * vec4(position, 1.0));
}
