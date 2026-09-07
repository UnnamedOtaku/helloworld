#include "Light.h"

int lightsCount = 0;

Light::Light(LightType type, glm::vec3 position, glm::vec3 target, float cOff, float oCOff, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
    : id(lightsCount), type(type), enabled(true), position(position), target(target), cutOff(glm::cos(glm::radians(cOff))), outerCutOff(glm::cos(glm::radians(oCOff))),
      ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic)
{
    lightsCount++;
}

void Light::Update(const Shader& shader) {
    shader.setBool("light[" + std::to_string(id) + "].enabled", enabled);
    shader.setInt("light[" + std::to_string(id) + "].type", static_cast<int>(type));
    shader.setVec3("light[" + std::to_string(id) + "].position", position);
    shader.setVec3("light[" + std::to_string(id) + "].target", target);
    shader.setFloat("light[" + std::to_string(id) + "].cutOff", cutOff);
    shader.setFloat("light[" + std::to_string(id) + "].outerCutOff", outerCutOff);
    shader.setVec3("light[" + std::to_string(id) + "].ambient", ambient / 255.0f);
    shader.setVec3("light[" + std::to_string(id) + "].diffuse", diffuse / 255.0f);
    shader.setVec3("light[" + std::to_string(id) + "].specular", specular / 255.0f);
    shader.setFloat("light[" + std::to_string(id) + "].constant", constant);
    shader.setFloat("light[" + std::to_string(id) + "].linear", linear);
    shader.setFloat("light[" + std::to_string(id) + "].quadratic", quadratic);
}

GpuLight Light::GetGpuData() const {
    return {
        glm::ivec4(static_cast<int>(type), enabled ? 1 : 0, 0, 0),
        glm::vec4(position, 0.0f),
        glm::vec4(target, 0.0f),
        glm::vec4(cutOff, outerCutOff, 0.0f, 0.0f),
        glm::vec4(ambient / 255.0f, 0.0f),
        glm::vec4(diffuse / 255.0f, 0.0f),
        glm::vec4(specular / 255.0f, 0.0f),
        glm::vec4(constant, linear, quadratic, 0.0f)
    };
}