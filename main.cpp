#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "resource_manager.h"
#include "NuklearRenderer.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

// Screen dimensions
const unsigned int SCREEN_WIDTH = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

// Global Game instance
Game CatChase(SCREEN_WIDTH, SCREEN_HEIGHT);

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(int argc, char* argv[]) {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path);
    exePath = exePath.substr(0, exePath.find_last_of("\\/"));
    _chdir(exePath.c_str());
#endif

    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CatChase", nullptr, nullptr);
    if (!window) {
        std::cerr << "❌ Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "❌ Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Nuklear
    NuklearRenderer nuklearGui(window);

    // Pass GUI to Game


    // Initialize game
    CatChase.Init();

    CatChase.SetUIRenderer(&nuklearGui);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        nuklearGui.BeginFrame();         // UI input
        CatChase.ProcessInput(window, deltaTime);
        CatChase.Update(deltaTime);

        glClearColor(0.2f, 0.2f, 0.2f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);

        CatChase.Render();
        CatChase.RenderUI();             // UI draw
        nuklearGui.EndFrame();           // UI render

        glfwSwapBuffers(window);
    }

    ResourceManager::Clear();
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            CatChase.Keys[key] = true;
        else if (action == GLFW_RELEASE)
            CatChase.Keys[key] = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    CatChase.SetSize(width, height);
}
