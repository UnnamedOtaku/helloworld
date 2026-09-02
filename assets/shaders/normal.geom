#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

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
    gl_Position = data_in[0].proj * gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = data_in[0].proj * (gl_in[0].gl_Position + 0.1f * vec4(data_in[0].vertexNormal, 0.0f));
    EmitVertex();
    EndPrimitive();

    gl_Position = data_in[1].proj * gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = data_in[1].proj * (gl_in[1].gl_Position + 0.1f * vec4(data_in[1].vertexNormal, 0.0f));
    EmitVertex();
    EndPrimitive();

    gl_Position = data_in[2].proj * gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = data_in[2].proj * (gl_in[2].gl_Position + 0.1f * vec4(data_in[2].vertexNormal, 0.0f));
    EmitVertex();
    EndPrimitive();
}