#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#define STB_IMAGE_IMPLEMENTATION
#include <memory>
#include <unordered_map>

#include "stb_image.h"

// Instantiate static variables
std::unordered_map<std::string, std::unique_ptr<Texture2D>> ResourceManager::Textures;
std::unordered_map<std::string, std::unique_ptr<Shader>>    ResourceManager::Shaders;

Shader& ResourceManager::LoadShader(const std::string_view vShaderFile,
                                    const std::string_view fShaderFile,
                                    const std::string_view gShaderFile,
                                    const std::string& name)
{
    auto shader = std::make_unique<Shader>(
        loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile)
    );
    Shader& ref = *shader;
    Shaders[name] = std::move(shader);
    return ref;
}

Shader& ResourceManager::GetShader(const std::string& name)
{
    return *Shaders.at(name);
}

Texture2D& ResourceManager::LoadTexture(const std::string_view file, bool alpha, const std::string& name)
{
    auto texture = std::make_unique<Texture2D>(
        loadTextureFromFile(file, alpha)
    );
    Texture2D& ref = *texture;
    Textures[name] = std::move(texture);
    return ref;
}

Texture2D& ResourceManager::GetTexture(const std::string& name)
{
    return *Textures.at(name);
}

void ResourceManager::Clear()
{
    Shaders.clear();   // std::unique_ptr handles deletion
    Textures.clear();
}

Shader ResourceManager::loadShaderFromFile(std::string_view vShaderFile,
                                           std::string_view fShaderFile,
                                           std::string_view gShaderFile)
{
    std::string vertexCode, fragmentCode, geometryCode;

    try {
        std::ifstream vertexShaderFileStream{std::string(vShaderFile)};
        std::ifstream fragmentShaderFileStream{std::string(fShaderFile)};

        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vertexShaderFileStream.rdbuf();
        fShaderStream << fragmentShaderFileStream.rdbuf();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (!gShaderFile.empty()) {
            std::ifstream geometryShaderFileStream{std::string(gShaderFile)};
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFileStream.rdbuf();
            geometryCode = gShaderStream.str();
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR::SHADER: Failed to read shader files: " << e.what() << "\n";
    }

    Shader shader;
    shader.Compile(vertexCode.c_str(), fragmentCode.c_str(), gShaderFile.empty() ? nullptr : geometryCode.c_str());
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(std::string_view file, bool alpha)
{
    Texture2D texture;
    if (alpha) {
        texture.SetFormat(GL_RGBA, GL_RGBA); // new setter (see below)
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(file.data(), &width, &height, &nrChannels, 0);
    if (data) {
        texture.Generate(width, height, data);
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << file << "\n";
    }
    return texture;
}
