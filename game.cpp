#include "game.h"

#include <unordered_set>
#include <cstdlib> // for rand()
#include <ctime>   // for seeding rand (e.g., in main())

#include "Dog.h"
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"
#include "Collision.h"
#include "LevelManager.h"
#include "TEXT_RENDERER.h"

#include "PauseMenu.h"
PauseMenu pauseMenu;
bool isPaused = false;




Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    ResourceManager::UnloadShader("pause");
    // OR
    ResourceManager::Clear(); // unloads ALL shaders/textures
}

void Game::Init() {

    srand(static_cast<unsigned>(time(nullptr)));
    ResourceManager::LoadShader("resources/shaders/pause.vert", "resources/shaders/pause.frag", nullptr, "pause");
    ResourceManager::LoadShader("resources/shaders/box.vert", "resources/shaders/box.frag", nullptr, "box");


    // Global TextRenderer
    auto text = ResourceManager::LoadTextRenderer("default", Width, Height);
    text->Load("resources/fonts/OCRAEXT.TTF", 20); // or your font
    auto textPause = ResourceManager::LoadTextRenderer("pause", Width, Height);
    textPause->Load("resources/fonts/OCRAEXT.TTF", 25); // or your font
    // Load level 0
    levelManager_.LoadLevel(0, Width, Height);
    pauseMenu.SetLevels({ "Level 1", "Level 2", "Level 3" });
}

void Game::Update(float dt)
{
	levelManager_.Update(dt);
}
void Game::ProcessInput(GLFWwindow* window, float dt)
{
    static bool pausePressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !pausePressed) {
        isPaused = !isPaused;
        pauseMenu.SetActive(isPaused);
        pausePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        pausePressed = false;
    }

    if (isPaused) {
        pauseMenu.HandleInput(window, dt,
            [&](PauseMenu::Option opt) {
                HandlePauseMenuSelection(opt, window);
            },
            [&](int level) {
                levelManager_.LoadLevel(level, Width, Height);
                isPaused = false;
                pauseMenu.SetActive(false);
            });

        return;
    }

    levelManager_.ProcessInput(dt, Keys);
}





void Game::Render()
{

	const glm::mat4& projection = levelManager_.GetCurrentLevel()->GetProjection();
	levelManager_.Render(projection);
	if (pauseMenu.IsActive()) {
		pauseMenu.Render(Width, Height);
		return; // skip rendering game
	}

}

void Game::SetSize(unsigned int width, unsigned int height)
{
	this->Width = width;
	this->Height = height;
}

void Game::HandlePauseMenuSelection(PauseMenu::Option opt, GLFWwindow* window) {
	switch (opt) {
		case PauseMenu::RESUME:
			isPaused = false;
			pauseMenu.SetActive(false);
			break;
		case PauseMenu::CHANGE_LEVEL:
			isPaused = true;

			break;
		case PauseMenu::QUIT:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
	}
}
