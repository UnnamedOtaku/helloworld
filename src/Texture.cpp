#include "Texture.h"

#include <iostream>
#include <stb_image.h>

Texture::Texture()
    : id(0), target(GL_TEXTURE_2D), unit(0), path(""), width(0), height(0), channels(0)
{
}

Texture::Texture(const std::string& filePath, GLenum target, GLuint unit)
    : id(0), target(target), unit(unit), path(filePath), width(0), height(0), channels(0)
{
    glGenTextures(1, &id);
    LoadFromFile(filePath);
}

Texture::~Texture()
{
    if (id != 0)
    {
        glDeleteTextures(1, &id);
        id = 0;
    }
}

bool Texture::LoadFromFile(const std::string& filePath, bool flipY)
{
    path = filePath;

    stbi_set_flip_vertically_on_load(flipY);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }

    if (id == 0)
        glGenTextures(1, &id);

    glBindTexture(target, id);

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(target);

    stbi_image_free(data);
    glBindTexture(target, 0);
    return true;
}

void Texture::Bind() const
{
    if (id == 0)
        return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, id);
}

void Texture::Unbind() const
{
    glBindTexture(target, 0);
}

void Texture::Update(const Shader& shader, const std::string& uniformName) const
{
    shader.setInt(uniformName, static_cast<int>(unit));
    Bind();
}
