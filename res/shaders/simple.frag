#version 430 core
in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

uniform vec3 camPos;
uniform layout(location = 6) vec4 lights[3];
uniform layout(location = 9) vec3 lightColors[3];
uniform vec3 ballPos;
uniform int drawMode;

float ballRadius = 3.0f;

in vec3 fragmentPos;
vec3 lightPos = vec3(0, 0, 0);

vec3 lightDirection = normalize(vec3(0.8, -0.5, 0.6));

float ambientStrenght = 0.1;
vec3 ambientColor = vec3(1, 1, 1);
vec3 ambient = ambientStrenght * ambientColor;

vec3 diffuseColor = vec3(1, 1, 1);
vec3 diffuse;


uniform sampler2D myTexture;
layout(binding = 1) uniform sampler2D charTexture;
out vec4 color;

float specularStrength = 0.4;
vec3 specularColor = vec3(1, 1, 1);
vec3 specular;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 reject(vec3 from, vec3 onto) {
	return from - onto*dot(from, onto)/dot(onto, onto);
}

float la = 0.1, lb = 10e-5, lc = 10e-4;
void main()
{
	if (drawMode == 1) {
		vec4 tex = texture(charTexture, textureCoordinates);
		/* color = vec4(1, 0, 0, tex.w != 0 ? 0 : 1); */
		color = vec4(tex.xyz, 1);
		/* color = vec4(1, 0, 0, 1); */
	}
	else {
	vec3 norm = normalize(normal);
	diffuse = vec3(0);
	specular = vec3(0);
	vec3 normalCol = vec3(0.5 * normal + 0.5);

	vec3 ballVec = ballPos - fragmentPos;

	for	(int i = 0; i < 3; i++) {
		lightPos = lights[i].xyz;

		vec3 lightVec = lightPos - fragmentPos;
		vec3 lightDir = normalize(lightVec);

		float r = length(reject(ballVec, lightVec));
		float shadow = 1;
		if (r < ballRadius) shadow =  pow(r / ballRadius, 2) + 1 / length(lightVec);
		if (length(ballVec) >= length(lightVec)) shadow = 1;
		/* float angle = acos(dot(ballVec, lightPos) */
		/* if (dot(ballVec, lightPos) < 0) shadow = 1; */
		if (normalize(ballVec) == -normalize(lightPos)) shadow = 1;
		
		float d = distance(lightPos, fragmentPos);
		float L = 1 / (la + d * lb + d * d * lc);

		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * lightColors[i] * L * shadow;

		vec3 viewDir = normalize(camPos - fragmentPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1024);
		specular += specularStrength * spec * specularColor * L * shadow;

	}

   	/* vec3 c = (ambient + diffuse + specular) * normalCol; // vec3(0.5 * normal + 0.5); */
   	vec3 c = (ambient + diffuse + specular) * vec3(1) + dither(textureCoordinates);
   	/* vec3 c = vec3(0.5 * normal + 0.5); */

	/* vec3 c = vec3(1, 1, 1) * max(0, dot(normal, -lightDir)); */
	color = vec4(c, 1.0);
	}
}
