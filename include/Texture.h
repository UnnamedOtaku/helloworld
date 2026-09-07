#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>
#include "Shader.h"

struct Texture {
    GLuint id;
    GLenum target;
    GLuint unit;
    std::string path;
    int width;
    int height;
    int channels;

    Texture();
    Texture(const std::string& filePath, GLenum target = GL_TEXTURE_2D, GLuint unit = 0);
    ~Texture();

    bool LoadFromFile(const std::string& filePath, bool flipY = true);
    void Bind() const;
    void Unbind() const;
    void Update(const Shader& shader, const std::string& uniformName) const;
};

#endif // TEXTURE_H
