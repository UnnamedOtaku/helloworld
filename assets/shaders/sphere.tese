#version 440 core

layout (triangles, equal_spacing, ccw) in;

in TCS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} tes_in[];

out vec3 vertexPos;
out vec3 vertexNormal;
out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float radius;

void main()
{
    vec3 objectPosition =
        gl_TessCoord.x * tes_in[0].position +
        gl_TessCoord.y * tes_in[1].position +
        gl_TessCoord.z * tes_in[2].position;

    vec3 spherePosition = normalize(objectPosition) * radius;
    vec4 worldPosition = model * vec4(spherePosition, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vertexPos = worldPosition.xyz;
    vertexNormal = normalize(normalMatrix * normalize(spherePosition));
    vertexColor =
        gl_TessCoord.x * tes_in[0].color +
        gl_TessCoord.y * tes_in[1].color +
        gl_TessCoord.z * tes_in[2].color;
    TexCoord =
        gl_TessCoord.x * tes_in[0].texCoord +
        gl_TessCoord.y * tes_in[1].texCoord +
        gl_TessCoord.z * tes_in[2].texCoord;

    gl_Position = proj * view * worldPosition;
}
