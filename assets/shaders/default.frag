#version 430 core

out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 TexCoord;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1
#define     LIGHT_SPOT              2

struct Light {
    ivec4 info;
    vec4 position;
    vec4 target;
    vec4 cutoffs;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
};

uniform vec3 viewPos;
  
uniform Material material;
uniform int lightCount;

layout(std430, binding = 0) readonly buffer LightBuffer {
    Light light[];
};

void main()
{
    vec3 norm = normalize(vertexNormal);
    vec3 viewDir = normalize(viewPos - vertexPos);
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount; i++)
    {
        if (light[i].info.y == 0) continue;
        vec3 lightPosition = light[i].position.xyz;
        vec3 lightTarget = light[i].target.xyz;
        int lightType = light[i].info.x;
        vec3 lightDir = lightType == LIGHT_DIRECTIONAL ? normalize(lightPosition - lightTarget) : normalize(lightPosition - vertexPos);

        float diff = max(dot(norm, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

        vec3 ambient = light[i].ambient.xyz * vec3(texture(material.diffuse, TexCoord));
        vec3 diffuse = light[i].diffuse.xyz * diff * vec3(texture(material.diffuse, TexCoord));
        vec3 specular = light[i].specular.xyz * spec * vec3(texture(material.specular, TexCoord)); 
        vec3 emission = vec3(texture(material.specular, TexCoord)) == vec3(0.0) ? vec3(texture(material.emission, TexCoord)) : vec3(0.0);

        if (lightType == LIGHT_SPOT)
        {
            float theta = dot(lightDir, normalize(lightPosition - lightTarget)); 
            float epsilon = (light[i].cutoffs.x - light[i].cutoffs.y);
            float intensity = clamp((theta - light[i].cutoffs.y) / epsilon, 0.0, 1.0);
            diffuse  *= intensity;
            specular *= intensity;
        }

        if (lightType != LIGHT_DIRECTIONAL)
        {
            float distance = length(lightPosition - vertexPos);
            float attenuation = 1.0 / (light[i].attenuation.x + light[i].attenuation.y * distance + light[i].attenuation.z * (distance * distance));

            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;
        }

        result += ambient + diffuse + specular + emission;
    }
    
    FragColor = vec4(result, 1.0);
}