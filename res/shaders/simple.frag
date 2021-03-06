#version 430 core
// in location
in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 uv;

// in vertex shader
in vec3 fragmentPos;
in mat3x3 TNB;

// out
out vec4 color;

// texture uniforms 
uniform sampler2D myTexture;
layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D roughnessMap;

// general uniforms
uniform layout(location = 6) vec4 lights[3];
uniform layout(location = 9) vec4 lightColors[3];
uniform layout(location = 12) vec4 color_in;

uniform vec3 camPos;
uniform int drawMode; // 0=3D, 1=2D

uniform vec3 ballPos;
float ballRadius = 3.0f;

vec3 norm;
vec4 baseColor;

vec3 lightPos = vec3(0, 0, 0);

float ambientStrenght = 0.1;
vec3 ambientColor = vec3(1, 1, 1);
vec3 ambient = ambientStrenght * ambientColor;

vec3 diffuseColor = vec3(1, 1, 1);
vec3 diffuse = vec3(0);

float la = 0.001, lb = 10e-3, lc = 10e-3;


float specularStrength = 0.4;
vec3 specularColor = vec3(1, 1, 1);
vec3 specular = vec3(0);
float roughness;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }
vec3 reject(vec3 from, vec3 onto) {	return from - onto*dot(from, onto)/dot(onto, onto); }

// Pasted from learnopengl
float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
// paste end

void main()
{

	switch (drawMode) {
		case 0: 
		case 3: 
			{
			norm = normalize(normal);
			baseColor = color_in;
			roughness = 64;
			}
			break;

		case 2:
			{
			norm = TNB * normalize(texture(normalMap, uv).rgb * 2 - 1);
			baseColor = texture(diffuseTexture, uv);
			roughness = 5.0f / pow(length(texture(roughnessMap, uv)), 2);
			}
			break;


	}


	vec3 ballVec = ballPos - fragmentPos;

	/* if (false)  //todo: remove */
	for	(int i = 0; i < 1; i++) {

		// Light variables
		lightPos = lights[i].xyz;
		vec3 lightVec = lightPos - fragmentPos;
		vec3 lightDir = normalize(lightVec);

	// Shadows 
	float r = length(reject(ballVec, lightVec));
		float shadow = 1;
		if (r < ballRadius) shadow =  pow(r / ballRadius, 2) + 1 / length(lightVec);
		if (length(ballVec) >= length(lightVec)) shadow = 1;
		if (normalize(ballVec) == -normalize(lightPos)) shadow = 1;
		
		// Distance dimming
		float d = distance(lightPos, fragmentPos);
		float L = 1 / (la + d * lb + d * d * lc);

		// Difuse
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * lightColors[i].xyz * L * shadow;

		// Specular
		vec3 viewDir = normalize(camPos - fragmentPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), roughness);
		specular += specularStrength * spec * specularColor * L * shadow;

	}

   	vec3 c = (ambient + diffuse + specular) * baseColor.xyz + dither(uv);
	/* float dist = 1 -  length(fragmentPos - camPos) / 10; */
	/* float depth = LinearizeDepth(gl_FragCoord.z) / far; */
	color = vec4(c, 1.0);
}
