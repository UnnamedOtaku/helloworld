#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    Shader();
    ~Shader();

    void add(const std::string& filePath, GLenum shaderType);
    void addFromString(const std::string& source, GLenum shaderType);
    bool link();
    void use();

    GLuint getID() const { return ID; }
    bool isLinked() const { return linked; }

    void setBool(const std::string &name, GLboolean value) const;
    void setInt(const std::string &name, GLint value) const;
    void setUInt(const std::string &name, GLuint value) const;
    void setFloat(const std::string &name, GLfloat value) const;
    void setDouble(const std::string &name, GLdouble value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setMat4(const std::string &name, const GLfloat* mat) const;

    void setVec2Array(const std::string &name, const glm::vec2* values, int count) const;
    void setVec3Array(const std::string &name, const glm::vec3* values, int count) const;
    void setVec4Array(const std::string &name, const glm::vec4* values, int count) const;
    void setMat4Array(const std::string &name, const glm::mat4* values, int count) const;
    void setFloatArray(const std::string &name, const float* values, int count) const;
    void setIntArray(const std::string &name, const int* values, int count) const;
    
    GLint getUniformLocation(const std::string &name) const;

    void clearShaders();

private:
    GLint ID;
    std::vector<GLuint> shaders;
    mutable std::unordered_map<std::string, GLint> uniformCache;
    bool linked;

    std::string readFile(const std::string &filePath);
    GLuint compileShader(const std::string &source, GLenum shaderType);
    void checkCompileErrors(GLuint shader, GLenum shaderType);
    GLint checkLinkErrors();
    void deleteShaders();

    std::string getShaderTypeName(GLenum shaderType);
};

#endif