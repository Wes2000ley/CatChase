#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <string_view>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    Shader();
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Disable copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader& Use();

    void Compile(std::string_view vertexSource,
                 std::string_view fragmentSource,
                 std::string_view geometrySource = {});

    void SetFloat(std::string_view name, float value, bool useShader = false);
    void SetInteger(std::string_view name, int value, bool useShader = false);
    void SetVector2f(std::string_view name, const glm::vec2& value, bool useShader = false);
    void SetVector3f(std::string_view name, const glm::vec3& value, bool useShader = false);
    void SetVector4f(std::string_view name, const glm::vec4& value, bool useShader = false);
    void SetMatrix4(std::string_view name, const glm::mat4& matrix, bool useShader = false);

    [[nodiscard]] unsigned int GetID() const { return ID; }

private:
    unsigned int ID = 0;

    void checkCompileErrors(unsigned int object, std::string_view type);
};

#endif // SHADER_H
