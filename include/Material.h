#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/glm.hpp>
#include "Shader.h"

struct Material {
    int diffuseTextureUnit;
    int specularTextureUnit;
    int emissionTextureUnit;
    float shininess;

    Material(int diffuseTextureUnit = 0, int specularTextureUnit = 1, int emissionTextureUnit = 2, float shininess = 64.0f);
    void Update(const Shader& shader, const std::string& name = "material") const;
};

#endif // MATERIAL_H
