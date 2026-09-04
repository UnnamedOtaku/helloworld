#version 440 core

layout (vertices = 3) out;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} tcs_in[];

out TCS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} tcs_out[];

uniform float tessellationLevel;

void main()
{
    tcs_out[gl_InvocationID].position = tcs_in[gl_InvocationID].position;
    tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
    tcs_out[gl_InvocationID].color = tcs_in[gl_InvocationID].color;
    tcs_out[gl_InvocationID].texCoord = tcs_in[gl_InvocationID].texCoord;

    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessellationLevel;
        gl_TessLevelOuter[1] = tessellationLevel;
        gl_TessLevelOuter[2] = tessellationLevel;
        gl_TessLevelInner[0] = tessellationLevel;
    }
}
