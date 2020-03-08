#version 430 core
in layout(location = 1) vec2 textureCoordinates;
out vec4 color;

layout(binding = 0) uniform sampler2D charTexture;

void main() {

		vec4 tex = texture(charTexture, textureCoordinates);
		/* color = vec4(1, 0, 0, tex.w != 0 ? 0 : 1); */
		color = vec4(tex.rgb, 1);
}


