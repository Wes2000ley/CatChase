#version 330 core
out vec4 FragColor;

uniform vec4 overlayColor;

void main()
{
    FragColor = overlayColor; // not sampled from a texture
}
