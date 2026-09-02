#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec3 vertexPos;
out vec3 vertexNormal;
out vec3 vertexColor;
out vec2 TexCoord;

in Data
{
    vec3 vertexPos;
    vec3 vertexNormal;
    vec3 vertexColor;
    vec2 TexCoord;
    mat4 proj;
} data_in[];

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        vertexPos = data_in[i].vertexPos;
        vertexNormal = data_in[i].vertexNormal;
        vertexColor = data_in[i].vertexColor;
        TexCoord = data_in[i].TexCoord;

        gl_Position = data_in[i].proj * (gl_in[i].gl_Position); // Desplazamiento de la posición del vértice
        EmitVertex();
    }
    EndPrimitive();
}