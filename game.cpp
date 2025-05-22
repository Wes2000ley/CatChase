#include "game.h"

Game::Game(unsigned int width, unsigned int height)
	: State(GameState::Active), Width(width), Height(height)
{
	// Keys is value-initialized by std::array
}

Game::~Game() = default;

void Game::Init()
{
}

void Game::Update(float dt)
{
}

void Game::ProcessInput(float dt)
{
}

void Game::Render()
{
}
