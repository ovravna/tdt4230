#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
uniform layout(location = 8) vec4 lights;

in vec3 fragmentPos;
vec3 lightPos = vec3(0, 0, 0);
vec3 diffuseColor = vec3(1, 1, 1);

vec3 lightDirection = normalize(vec3(0.8, -0.5, 0.6));

float ambientStrenght = 0.2;
vec3 ambientColor = vec3(0.8, 0.1, 0.6);
vec3 ambient = ambientStrenght * ambientColor;


uniform sampler2D myTexture;
out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
	lightPos = lights.xyz;
 	vec3 lightDir = normalize(lightPos - fragmentPos);
	vec3 norm = normalize(normal);
	float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
	vec3 diffuse = diff * diffuseColor;

   	vec3 c = (ambient + diffuse) * vec3(1, 1, 1); // vec3(0.5 * normal + 0.5);
   	/* vec3 c = vec3(0.5 * normal + 0.5); */

	/* vec3 c = vec3(1, 1, 1) * max(0, dot(normal, -lightDir)); */
	color = vec4(c, 1.0);
}
