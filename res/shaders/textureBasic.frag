#version 430 core
out vec4 FragColor;

in layout(location=1) vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}
