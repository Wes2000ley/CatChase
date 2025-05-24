
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include <glad/glad.h>

#include "texture.h"
#include "shader.h"
#include "TEXT_RENDERER.h"


// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no 
// public constructor is defined.
class ResourceManager
{
public:
    // resource storage
    static std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
    static std::unordered_map<std::string, std::shared_ptr<Texture2D>> Textures;
    // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
    static std::shared_ptr<Shader> LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, const std::string &name);
    // retrieves a stored sader
    static std::shared_ptr<Shader> GetShader(const std::string &name);
    // loads (and generates) a texture from file
    static std::shared_ptr<Texture2D> LoadTexture(const char *file, bool alpha, const std::string &name);
    // retrieves a stored texture
    static std::shared_ptr<Texture2D> GetTexture(const std::string &name);
    // properly de-allocates all loaded resources
    static void      Clear();

static std::map<std::string, std::shared_ptr<TextRenderer>> TextRenderers;
    static std::shared_ptr<TextRenderer> LoadTextRenderer(const std::string& name, unsigned int width, unsigned int height);
static TextRenderer& GetTextRenderer(const std::string& name);
    static void UnloadTexture(const std::string& name);
    static void UnloadShader(const std::string& name);

private:
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() { }
    // loads and generates a shader from file
    static Shader    loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile = nullptr);
    // loads a single texture from file
    static Texture2D loadTextureFromFile(const char *file, bool alpha);
};



#endif
