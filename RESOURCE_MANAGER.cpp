#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Instantiate static variables
std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::Textures;
std::unordered_map<std::string, std::shared_ptr<Shader>>    ResourceManager::Shaders;
std::map<std::string, std::shared_ptr<TextRenderer>>        ResourceManager::TextRenderers;
static std::unordered_map<std::string, std::string>         texturePaths;

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const char* file, bool alpha, const std::string& name) {
    std::string newPath = file;

    if (Textures.find(name) != Textures.end()) {
        if (texturePaths[name] == newPath)
            return Textures[name]; // same file, skip reload
    }

    texturePaths[name] = newPath;

    auto texture = std::make_shared<Texture2D>();
    if (alpha) {
        texture->Internal_Format = GL_RGBA;
        texture->Image_Format = GL_RGBA;
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "❌ Failed to load texture: " << file << std::endl;
        return nullptr;
    }

    texture->Generate(width, height, data);
    stbi_image_free(data);
    Textures[name] = texture;

    return texture;
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name) {
    auto shader = std::make_shared<Shader>();
    *shader = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    Shaders[name] = shader;
    return shader;
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string &name) {
    auto it = Textures.find(name);
    if (it != Textures.end()) return it->second;
    std::cerr << "⚠️ Texture not found: " << name << std::endl;
    return nullptr;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string &name) {
    auto it = Shaders.find(name);
    if (it != Shaders.end()) return it->second;
    std::cerr << "⚠️ Shader not found: " << name << std::endl;
    return nullptr;
}

void ResourceManager::Clear() {
    for (auto& [name, shader] : Shaders)
        if (shader && shader->ID) glDeleteProgram(shader->ID);
    for (auto& [name, texture] : Textures)
        if (texture && texture->ID) glDeleteTextures(1, &texture->ID);

    Shaders.clear();
    Textures.clear();
    TextRenderers.clear();
    texturePaths.clear();
}

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile) {
    std::string vertexCode, fragmentCode, geometryCode;

    try {
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        vertexShaderFile.close();
        fragmentShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception& e) {
        std::cerr << "ERROR::SHADER: Failed to read shader files: " << e.what() << std::endl;
    }

    Shader shader;
    shader.Compile(
        vertexCode.c_str(),
        fragmentCode.c_str(),
        gShaderFile ? geometryCode.c_str() : nullptr
    );
    return shader;
}

std::shared_ptr<TextRenderer> ResourceManager::LoadTextRenderer(const std::string& name, unsigned int width, unsigned int height) {
    auto renderer = std::make_shared<TextRenderer>(width, height);
    TextRenderers[name] = renderer;
    return renderer;
}

TextRenderer& ResourceManager::GetTextRenderer(const std::string& name) {
    auto it = TextRenderers.find(name);
    if (it != TextRenderers.end()) return *it->second;
    throw std::runtime_error("TextRenderer '" + name + "' not found");
}

void ResourceManager::UnloadTexture(const std::string& name) {
    auto it = Textures.find(name);
    if (it != Textures.end()) {
        if (it->second && it->second->ID)
            glDeleteTextures(1, &it->second->ID);
        Textures.erase(it);
    }
}

void ResourceManager::UnloadShader(const std::string& name) {
    auto it = Shaders.find(name);
    if (it != Shaders.end()) {
        if (it->second && it->second->ID)
            glDeleteProgram(it->second->ID);
        Shaders.erase(it);
    }
}
