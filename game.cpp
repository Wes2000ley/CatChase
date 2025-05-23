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

	levelManager_.LoadLevel(0, Width, Height);



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
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pauseMenu.Navigate(-1);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pauseMenu.Navigate(+1);

		static bool enterPressed = false;
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
			pauseMenu.Select([&](PauseMenu::Option opt) {
				switch (opt) {
					case PauseMenu::RESUME:
						isPaused = false;
						pauseMenu.SetActive(false);
						break;
					case PauseMenu::CHANGE_LEVEL:
						levelManager_.LoadLevel(1, Width, Height); // Example change
						isPaused = false;
						pauseMenu.SetActive(false);
						break;
					case PauseMenu::QUIT:
						glfwSetWindowShouldClose(window, GLFW_TRUE);
						break;
				}
			});
			enterPressed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
			enterPressed = false;
		}

		return; // skip game input
	}

	levelManager_.ProcessInput(dt, Keys);
}



void Game::Render()
{
	if (pauseMenu.IsActive()) {
		pauseMenu.Render();
		return; // skip rendering game
	}

	const glm::mat4& projection = levelManager_.GetCurrentLevel()->GetProjection();
	levelManager_.Render(projection);


}

void Game::SetSize(unsigned int width, unsigned int height)
{
	this->Width = width;
	this->Height = height;
}
