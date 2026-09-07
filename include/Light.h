#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "Shader.h"

struct GpuLight {
    glm::ivec4 info;
    glm::vec4 position;
    glm::vec4 target;
    glm::vec4 cutoffs;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 attenuation;
};

enum LightType {
    DIRECTIONAL,
    POINT,
    SPOT
};

extern int lightsCount;

struct Light {
    int id;
    LightType type;
    bool enabled;
    glm::vec3 position;
    glm::vec3 target;
    float cutOff;
    float outerCutOff;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
    
    Light(LightType type, glm::vec3 position, glm::vec3 target, float cOff, float oCOff, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);
    void Update(const Shader& shader);
    GpuLight GetGpuData() const;
};

#endif // LIGHT_H