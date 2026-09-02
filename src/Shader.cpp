#include "Shader.h"

Shader::Shader() : ID(0), linked(false) {}

Shader::~Shader()
{
    deleteShaders();
    if( ID != 0 )
    {
        glDeleteProgram(ID);
    }
}

void Shader::add(const std::string &filePath, GLenum shaderType)
{
    std::string source = readFile(filePath);
    if( source.empty() )
    {
        std::cerr << "ERROR::" << getShaderTypeName(shaderType) << "::SHADER::FAILED_TO_READ_SHADER_FILE: " << filePath << std::endl;
        return;
    }
    addFromString(source, shaderType);
}

void Shader::addFromString(const std::string &source, GLenum shaderType)
{
    if( linked )
    {
        std::cerr << "SHADER::" << getShaderTypeName(shaderType) << "SHADER::CANNOT_ADD_SHADER_AFTER_LINKING" << std::endl;
        return;
    }

    GLint shader = compileShader(source, shaderType);
    if( shader != 0 )
    {
        shaders.push_back(shader);
    }
}

std::string Shader::readFile(const std::string &filePath)
{
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if( !fileStream.is_open() )
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
        return "";
    }

    std::stringstream sstr;
    sstr << fileStream.rdbuf();
    content = sstr.str();
    fileStream.close();

    return content;
}

GLuint Shader::compileShader(const std::string &source, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    const GLchar* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, NULL);
    glCompileShader(shader);

    checkCompileErrors(shader, shaderType);
    return shader;
}

void Shader::checkCompileErrors(GLuint shader, GLenum shaderType)
{
    GLint success;
    GLchar infoLog[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if( !success )
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr
        << "ERROR::"
        << getShaderTypeName(shaderType)
        << "::SHADER::COMPILATION_ERROR:\n"
        << infoLog
        << "\n -- --------------------------------------------------- -- "
        << std::endl;
    }
}

GLint Shader::checkLinkErrors()
{
    GLint success;
    GLchar infoLog[1024];

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if( !success )
    {
        glGetProgramInfoLog(ID, 1024, NULL, infoLog);
        std::cerr
        << "ERROR::PROGRAM::LINKING_ERROR:\n"
        << infoLog
        << "\n -- --------------------------------------------------- -- "
        << std::endl;
    }

    return success;
}

void Shader::deleteShaders()
{
    for( GLuint shader : shaders )
    {
        glDetachShader(ID, shader);
        glDeleteShader(shader);
    }
    shaders.clear();
}

bool Shader::link()
{
    if( shaders.empty() )
    {
        std::cerr
        << "ERROR::PROGRAM::NO_SHADER_ATTACHED"
        << std::endl;
        return false;
    }

    ID = glCreateProgram();

    for( GLuint shader : shaders )
    {
        glAttachShader(ID, shader);
    }

    glLinkProgram(ID);
    GLint success = checkLinkErrors();
    if ( !success ) return false;

    deleteShaders();
    linked = true;

    uniformCache.clear();

    return true;
}

void Shader::use() 
{ 
    if (!linked) {
        std::cerr << "WARNING::SHADER::CANNOT_USE_SHADER_BEFORE_LINKING" << std::endl;
        return;
    }
    glUseProgram(ID);
}

void Shader::clearShaders() {
    if (!linked) {
        deleteShaders();
    } else {
        std::cerr << "WARNING::SHADER::CANNOT_CLEAR_SHADERS_AFTER_LINKING" << std::endl;
    }
}

std::string Shader::getShaderTypeName(GLenum shaderType) {
    switch (shaderType) {
        case GL_VERTEX_SHADER: return "VERTEX";
        case GL_FRAGMENT_SHADER: return "FRAGMENT";
        case GL_GEOMETRY_SHADER: return "GEOMETRY";
        case GL_TESS_CONTROL_SHADER: return "TESS_CONTROL";
        case GL_TESS_EVALUATION_SHADER: return "TESS_EVALUATION";
        case GL_COMPUTE_SHADER: return "COMPUTE";
        default: return "UNKNOWN";
    }
}

GLint Shader::getUniformLocation(const std::string &name) const {
    if (!linked) {
        std::cerr << "WARNING::SHADER::CANNOT_GET_UNIFORM_LOCATION_BEFORE_LINKING" << std::endl;
        return -1;
    }
    
    // Buscar en cache
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        return it->second;
    }
    
    // Si no está en cache, obtener y guardar
    GLint location = glGetUniformLocation(ID, name.c_str());
    uniformCache[name] = location;
    
    if (location == -1) {
        std::cerr << "WARNING::SHADER::UNIFORM_NOT_FOUND: " << name << std::endl;
    }
    
    return location;
}

void Shader::setBool(const std::string &name, GLboolean value) const {
    glUniform1i(getUniformLocation(name), (int)value);
}

void Shader::setInt(const std::string &name, GLint value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setUInt(const std::string &name, GLuint value) const {
    glUniform1ui(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string &name, GLfloat value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setDouble(const std::string &name, GLdouble value) const {
    glUniform1d(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(getUniformLocation(name), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string &name, const GLfloat* mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, mat);
}

void Shader::setVec2Array(const std::string &name, const glm::vec2* values, int count) const {
    glUniform2fv(getUniformLocation(name), count, glm::value_ptr(values[0]));
}

void Shader::setVec3Array(const std::string &name, const glm::vec3* values, int count) const {
    glUniform3fv(getUniformLocation(name), count, glm::value_ptr(values[0]));
}

void Shader::setVec4Array(const std::string &name, const glm::vec4* values, int count) const {
    glUniform4fv(getUniformLocation(name), count, glm::value_ptr(values[0]));
}

void Shader::setMat4Array(const std::string &name, const glm::mat4* values, int count) const {
    glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::setFloatArray(const std::string &name, const float* values, int count) const {
    glUniform1fv(getUniformLocation(name), count, values);
}

void Shader::setIntArray(const std::string &name, const int* values, int count) const {
    glUniform1iv(getUniformLocation(name), count, values);
}