#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <glad/glad.h>

#include "texture.h"
#include "shader.h"

// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. All resources are stored
// and managed internally via smart pointers to prevent leaks.
class ResourceManager
{
public:
    // Loads and compiles a shader program from file.
    // Optionally includes a geometry shader. Returns a reference to the stored Shader.
    static Shader& LoadShader(std::string_view vShaderFile,
                              std::string_view fShaderFile,
                              std::string_view gShaderFile,
                              const std::string& name);

    // Retrieves a previously loaded Shader
    static Shader& GetShader(const std::string& name);

    // Loads and creates a texture from file. Returns a reference to the stored Texture2D.
    static Texture2D& LoadTexture(std::string_view file, bool alpha, const std::string& name);

    // Retrieves a previously loaded Texture2D
    static Texture2D& GetTexture(const std::string& name);

    // Clears all loaded resources
    static void Clear();

private:
    ResourceManager() = default;

    static Shader loadShaderFromFile(std::string_view vShaderFile,
                                     std::string_view fShaderFile,
                                     std::string_view gShaderFile = {});

    static Texture2D loadTextureFromFile(std::string_view file, bool alpha);

    static std::unordered_map<std::string, std::unique_ptr<Shader>>    Shaders;
    static std::unordered_map<std::string, std::unique_ptr<Texture2D>> Textures;
};

#endif
