#version 330 core

out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 TexCoord;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}