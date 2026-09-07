#include "Material.h"

Material::Material(int diffuseTextureUnit, int specularTextureUnit, int emissionTextureUnit, float shininess)
    : diffuseTextureUnit(diffuseTextureUnit),
      specularTextureUnit(specularTextureUnit),
      emissionTextureUnit(emissionTextureUnit),
      shininess(shininess)
{
}

void Material::Update(const Shader& shader, const std::string& name) const
{
    shader.setInt(name + ".diffuse", diffuseTextureUnit);
    shader.setInt(name + ".specular", specularTextureUnit);
    shader.setInt(name + ".emission", emissionTextureUnit);
    shader.setFloat(name + ".shininess", shininess);
}
