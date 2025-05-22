#include "shader.h"
#include <iostream>

Shader::Shader() = default;

Shader::~Shader()
{
    if (ID != 0)
        glDeleteProgram(ID);
}

Shader::Shader(Shader&& other) noexcept
{
    ID = other.ID;
    other.ID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        if (ID != 0)
            glDeleteProgram(ID);
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

Shader& Shader::Use()
{
    glUseProgram(ID);
    return *this;
}

void Shader::Compile(std::string_view vertexSource,
                     std::string_view fragmentSource,
                     std::string_view geometrySource)
{
    unsigned int sVertex = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSource.data();
    glShaderSource(sVertex, 1, &vSrc, nullptr);
    glCompileShader(sVertex);
    checkCompileErrors(sVertex, "VERTEX");

    unsigned int sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSource.data();
    glShaderSource(sFragment, 1, &fSrc, nullptr);
    glCompileShader(sFragment);
    checkCompileErrors(sFragment, "FRAGMENT");

    unsigned int gShader = 0;
    if (!geometrySource.empty())
    {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        const char* gSrc = geometrySource.data();
        glShaderSource(gShader, 1, &gSrc, nullptr);
        glCompileShader(gShader);
        checkCompileErrors(gShader, "GEOMETRY");
    }

    ID = glCreateProgram();
    glAttachShader(ID, sVertex);
    glAttachShader(ID, sFragment);
    if (gShader)
        glAttachShader(ID, gShader);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (gShader)
        glDeleteShader(gShader);
}

void Shader::SetFloat(std::string_view name, float value, bool useShader)
{
    if (useShader) Use();
    glUniform1f(glGetUniformLocation(ID, name.data()), value);
}
void Shader::SetInteger(std::string_view name, int value, bool useShader)
{
    if (useShader) Use();
    glUniform1i(glGetUniformLocation(ID, name.data()), value);
}
void Shader::SetVector2f(std::string_view name, const glm::vec2& value, bool useShader)
{
    if (useShader) Use();
    glUniform2f(glGetUniformLocation(ID, name.data()), value.x, value.y);
}
void Shader::SetVector3f(std::string_view name, const glm::vec3& value, bool useShader)
{
    if (useShader) Use();
    glUniform3f(glGetUniformLocation(ID, name.data()), value.x, value.y, value.z);
}
void Shader::SetVector4f(std::string_view name, const glm::vec4& value, bool useShader)
{
    if (useShader) Use();
    glUniform4f(glGetUniformLocation(ID, name.data()), value.x, value.y, value.z, value.w);
}
void Shader::SetMatrix4(std::string_view name, const glm::mat4& matrix, bool useShader)
{
    if (useShader) Use();
    glUniformMatrix4fv(glGetUniformLocation(ID, name.data()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::checkCompileErrors(unsigned int object, std::string_view type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cerr << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cerr << "| ERROR::SHADER: Link-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
