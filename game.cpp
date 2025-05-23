#include "game.h"

#include <unordered_set>

#include "Dog.h"
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"
#include "Collision.h"
#include "LevelManager.h"
#include "TEXT_RENDERER.h"0

#include "PauseMenu.h"
PauseMenu pauseMenu;
bool isPaused = false;




Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
}

void Game::Init()
{

	ResourceManager::LoadShader("resources/shaders/pause.vert", "resources/shaders/pause.frag", nullptr, "pause");
	levelManager_.LoadLevel(0, Width, Height);
	auto& text = ResourceManager::LoadTextRenderer("default", Width, Height);
	text.Load("resources/fonts/OCRAEXT.TTF", 24); // or your font path
}

void Game::Update(float dt)
{
	levelManager_.Update(dt);
}
void Game::ProcessInput(GLFWwindow* window, float dt)
{
	// Toggle pause
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
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float centerX = Width / 2.0f;
		TextRenderer& text = ResourceManager::GetTextRenderer("default");
		float scaleOption = 1.5f;

		for (int i = 0; i < PauseMenu::COUNT; ++i) {
			std::string label = pauseMenu.GetOptionLabel(i);
			float optionWidth = text.MeasureTextWidth(label, scaleOption);
			float optionHeight = 30.0f;  // approximate height
			float optionX = centerX - optionWidth / 2.0f;
			float optionY = pauseMenu.GetOptionY(i, Height);

			if (mouseX >= optionX && mouseX <= optionX + optionWidth &&
				mouseY >= optionY && mouseY <= optionY + optionHeight) {
				pauseMenu.SetSelectedIndex(i);
				break;
				}
		}

		// Handle mouse hover
		int hovered = -1;


		for (int i = 0; i < PauseMenu::COUNT; ++i) {
			auto bounds = pauseMenu.GetOptionBounds(i, Width, Height, scaleOption);

			if (mouseX >= bounds.x && mouseX <= bounds.x + bounds.width &&
				mouseY >= bounds.y && mouseY <= bounds.y + bounds.height) {
				pauseMenu.SetSelectedIndex(i);
				break;
				}
		}

		if (hovered != -1) pauseMenu.SetSelectedIndex(hovered);

		// Handle mouse click
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			pauseMenu.Select([&](PauseMenu::Option opt) {
				HandlePauseMenuSelection(opt, window);
			});
			return;
		}

		// Handle key navigation
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			pauseMenu.Navigate(-1);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			pauseMenu.Navigate(+1);

		static bool enterPressed = false;
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
			pauseMenu.Select([&](PauseMenu::Option opt) {
				HandlePauseMenuSelection(opt, window);
			});
			enterPressed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
			enterPressed = false;
		}

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
			levelManager_.LoadLevel(1, Width, Height);
			isPaused = false;
			pauseMenu.SetActive(false);
			break;
		case PauseMenu::QUIT:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
	}
}
