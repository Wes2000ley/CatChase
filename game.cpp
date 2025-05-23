#include "game.h"

#include <unordered_set>

#include "Dog.h"
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"
#include "Collision.h"
#include "LevelManager.h"





Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
}

void Game::Init()
{
	levelManager_.LoadLevel(1, Width, Height);



}

void Game::Update(float dt)
{
	levelManager_.Update(dt);
}
void Game::ProcessInput(float dt)
{
	levelManager_.ProcessInput(dt, Keys);
}


void Game::Render()
{
	const glm::mat4& projection = levelManager_.GetCurrentLevel()->GetProjection();
	levelManager_.Render(projection);


}

void Game::SetSize(unsigned int width, unsigned int height)
{
	this->Width = width;
	this->Height = height;
}
