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


    // Global TextRenderer
    auto text = ResourceManager::LoadTextRenderer("default", Width, Height);
    text->Load("resources/fonts/OCRAEXT.TTF", 20); // or your font
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
    static bool mousePressed = false;  // 👈 Move this here
    static bool enterPressed = false;  // 👈 Shared across modes

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !pausePressed) {
        isPaused = !isPaused;
        pauseMenu.SetActive(isPaused);
        pausePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        pausePressed = false;
    }

    if (isPaused) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        TextRenderer& text = ResourceManager::GetTextRenderer("default");
        float scaleOption = 1.5f;

        if (pauseMenu.IsInLevelSelectMode()) {
            // Mouse hover
            for (int i = 0; i < pauseMenu.GetLevelCount(); ++i) {
                auto bounds = pauseMenu.GetLevelBounds(i, Width, Height, scaleOption);
                if (mouseX >= bounds.x && mouseX <= bounds.x + bounds.width &&
                    mouseY >= bounds.y && mouseY <= bounds.y + bounds.height) {
                    pauseMenu.SetSelectedLevel(i);
                    break;
                }
            }

            // Mouse click on level
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
                int levelToLoad = pauseMenu.GetSelectedLevel();
                levelManager_.LoadLevel(levelToLoad, Width, Height);
                pauseMenu.SetActive(false);
                isPaused = false;
                mousePressed = true;
                return;
            }
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                mousePressed = false;
            }

            // Arrow keys
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                pauseMenu.NavigateLevels(-1);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                pauseMenu.NavigateLevels(+1);

            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
                int levelToLoad = pauseMenu.GetSelectedLevel();
                levelManager_.LoadLevel(levelToLoad, Width, Height);
                pauseMenu.SetActive(false);
                isPaused = false;
                enterPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
                enterPressed = false;

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                pauseMenu.ExitLevelSelect();

            return;
        }

        // --- MAIN pause menu ---
        for (int i = 0; i < PauseMenu::COUNT; ++i) {
            auto bounds = pauseMenu.GetOptionBounds(i, Width, Height, scaleOption);
            if (mouseX >= bounds.x && mouseX <= bounds.x + bounds.width &&
                mouseY >= bounds.y && mouseY <= bounds.y + bounds.height) {
                pauseMenu.SetSelectedIndex(i);
                break;
            }
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
            PauseMenu::Option selected;
            bool selectionMade = false;

            pauseMenu.Select([&](PauseMenu::Option opt) {
                selected = opt;
                selectionMade = true;
            });

            if (selectionMade && !pauseMenu.IsInLevelSelectMode()) {
                HandlePauseMenuSelection(selected, window);
            }

            mousePressed = true;
            return;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
            mousePressed = false;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            pauseMenu.Navigate(-1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            pauseMenu.Navigate(+1);

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
            pauseMenu.Select([&](PauseMenu::Option opt) {
                HandlePauseMenuSelection(opt, window);
            });
            enterPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
            enterPressed = false;

        return;
    }

    levelManager_.ProcessInput(dt, Keys);
}





void Game::Render()
{

	const glm::mat4& projection = levelManager_.GetCurrentLevel()->GetProjection();
	levelManager_.Render(projection);
	if (pauseMenu.IsActive()) {
		pauseMenu.Render();
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
