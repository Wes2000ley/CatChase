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
	GUI = new NuklearRenderer(glfwGetCurrentContext()); // or pass `window` if you store it


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
void Game::RenderUI() {
	if (!GUI) return;

	struct nk_context* ctx = GUI->GetContext();
	if (pauseMenu.IsActive()) {
		if (nk_begin(ctx, "Pause Menu", nk_rect(50, 50, 220, 200),
					 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_button_label(ctx, "Resume")) {
				isPaused = false;
				pauseMenu.SetActive(false);
			}
			if (nk_button_label(ctx, "Change Level")) {
				// You can pop a submenu here
			}
			if (nk_button_label(ctx, "Quit")) {
				glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
			}
					 }
		nk_end(ctx);
	}
}
void Game::SetUIRenderer(NuklearRenderer* gui) {
	GUI = gui;
}
