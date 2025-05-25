#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
//#include <GL/gl.h>
#include <glad/glad.h>

#include "stb_image.h"

// Static maps
std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::Textures;
std::unordered_map<std::string, std::shared_ptr<Shader>>    ResourceManager::Shaders;
std::map<std::string, std::shared_ptr<TextRenderer>>        ResourceManager::TextRenderers;

// Track file paths for reload detection
static std::unordered_map<std::string, std::string> texturePaths;
static std::unordered_map<std::string, std::pair<std::string, std::string>> shaderPaths;

// Load Texture2D
std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const char* file, bool alpha, const std::string& name) {
    std::string newPath = file;

    auto it = Textures.find(name);
    if (it != Textures.end()) {
        if (texturePaths[name] == newPath)
            return it->second;

        if (it->second && it->second->ID != 0)
            glDeleteTextures(1, &it->second->ID);

        Textures.erase(it);
        texturePaths.erase(name);
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "❌ Failed to load texture: " << file << std::endl;
        return nullptr;
    }

    auto texture = std::make_shared<Texture2D>();
    if (alpha) {
        texture->Internal_Format = GL_RGBA;
        texture->Image_Format = GL_RGBA;
    }

    texture->Generate(width, height, data);
    stbi_image_free(data);

    Textures[name] = texture;
    texturePaths[name] = newPath;
    return texture;
}

// Load Shader
std::shared_ptr<Shader> ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name) {
    std::string vsPath = vShaderFile;
    std::string fsPath = fShaderFile;

    auto it = Shaders.find(name);
    if (it != Shaders.end()) {
        auto& [oldVS, oldFS] = shaderPaths[name];
        if (oldVS == vsPath && oldFS == fsPath)
            return it->second;

        if (it->second && it->second->ID != 0)
            glDeleteProgram(it->second->ID);

        Shaders.erase(it);
        shaderPaths.erase(name);
    }

    auto shader = std::make_shared<Shader>();
    *shader = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    Shaders[name] = shader;
    shaderPaths[name] = {vsPath, fsPath};
    return shader;
}

// Getters
std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& name) {
    auto it = Textures.find(name);
    if (it != Textures.end()) return it->second;
    std::cerr << "⚠️ Texture not found: " << name << std::endl;
    return nullptr;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name) {
    auto it = Shaders.find(name);
    if (it != Shaders.end()) return it->second;
    std::cerr << "⚠️ Shader not found: " << name << std::endl;
    return nullptr;
}

// Clear all
void ResourceManager::Clear() {
    for (auto& [_, shader] : Shaders)
        if (shader && shader->ID != 0)
            glDeleteProgram(shader->ID);

    for (auto& [_, texture] : Textures)
        if (texture && texture->ID != 0)
            glDeleteTextures(1, &texture->ID);

    Shaders.clear();
    Textures.clear();
    TextRenderers.clear();
    texturePaths.clear();
    shaderPaths.clear();
}

// Load shader source and compile
Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    std::string vertexCode, fragmentCode, geometryCode;

    try {
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (gShaderFile) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception& e) {
        std::cerr << "❌ ERROR::SHADER: Failed to read files: " << e.what() << std::endl;
    }

    Shader shader;
    shader.Compile(
        vertexCode.c_str(),
        fragmentCode.c_str(),
        gShaderFile ? geometryCode.c_str() : nullptr
    );
    return shader;
}

// Load and replace TextRenderer safely
std::shared_ptr<TextRenderer> ResourceManager::LoadTextRenderer(const std::string& name, unsigned int width, unsigned int height) {
    auto it = TextRenderers.find(name);
    if (it != TextRenderers.end()) {
        TextRenderers.erase(it); // 🧹 Remove old
    }

    auto renderer = std::make_shared<TextRenderer>(width, height);
    TextRenderers[name] = renderer;
    return renderer;
}

TextRenderer& ResourceManager::GetTextRenderer(const std::string& name) {
    auto it = TextRenderers.find(name);
    if (it != TextRenderers.end()) return *it->second;
    throw std::runtime_error("TextRenderer '" + name + "' not found");
}

// Manual unloads
void ResourceManager::UnloadTexture(const std::string& name) {
    auto it = Textures.find(name);
    if (it != Textures.end()) {
        if (it->second && it->second->ID != 0)
            glDeleteTextures(1, &it->second->ID);
        Textures.erase(it);
        texturePaths.erase(name);
    }
}

void ResourceManager::UnloadShader(const std::string& name) {
    auto it = Shaders.find(name);
    if (it != Shaders.end()) {
        if (it->second && it->second->ID != 0)
            glDeleteProgram(it->second->ID);
        Shaders.erase(it);
        shaderPaths.erase(name);
    }
}
std::shared_ptr<TextRenderer> ResourceManager::GetTextRendererPtr(const std::string& name) {
    auto it = TextRenderers.find(name);
    if (it != TextRenderers.end()) return it->second;
    throw std::runtime_error("TextRenderer '" + name + "' not found");
}
