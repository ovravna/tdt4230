#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 2) vec2 textureCoordinates_in;

uniform mat4 orthoProjection;
uniform vec3 textPos;

out layout(location = 1) vec2 textureCoordinates_out;

void main()
{
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = orthoProjection * vec4(textPos + position, 1.0f);
}
