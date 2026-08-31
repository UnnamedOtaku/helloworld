#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out vec3 vertexPos;
out vec3 vertexNormal;
out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vertexPos = vec3(worldPos);
    
    // Transformación correcta de normales (escala y rotación)
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vertexNormal = normalMatrix * aNormal;
    
    vertexColor = aColor;
    TexCoord = aTexCoord;
    
    gl_Position = proj * view * worldPos;
}