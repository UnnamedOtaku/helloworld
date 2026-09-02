#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out Data
{
    vec3 vertexPos;
    vec3 vertexNormal;
    vec3 vertexColor;
    vec2 TexCoord;
    mat4 proj;
} data_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    data_out.vertexPos = vec3(worldPos);
    
    // Transformación correcta de normales (escala y rotación)
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    data_out.vertexNormal = normalize(normalMatrix * aNormal);
    
    data_out.vertexColor = aColor;
    data_out.TexCoord = aTexCoord;

    data_out.proj = proj * view;
    
    gl_Position = worldPos;
}