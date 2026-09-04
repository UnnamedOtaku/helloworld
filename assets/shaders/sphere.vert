#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out VS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} vs_out;

void main()
{
    vs_out.position = aPos;
    vs_out.normal = aNormal;
    vs_out.color = aColor;
    vs_out.texCoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0);
}
