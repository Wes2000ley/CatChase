/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "Dog.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

void Game::Init() {
	// Load shader and texture for dog
	ResourceManager::LoadShader("resources/shaders/dogsprite.vert", "resources/shaders/dogsprite.frag", nullptr, "sprite");
	ResourceManager::LoadTexture("resources/textures/48DogSpriteSheet.png", true, "dog");

	// Load slime texture
	ResourceManager::LoadTexture("resources/textures/Slime.png", true, "slime");

	// Setup shader projection
	Shader shader = ResourceManager::GetShader("sprite");
	shader.Use();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.0f, -1.0f, 1.0f);
	shader.SetMatrix4("projection", projection);

	// Create dog
	Texture2D dogTexture = ResourceManager::GetTexture("dog");
	dog_ = new Dog(shader, dogTexture, glm::vec2(100, 100), glm::ivec2(2, 3));

	// Create slime enemy
	Texture2D slimeTexture = ResourceManager::GetTexture("slime");
	slime1_ = new Enemy(shader, slimeTexture, glm::vec2(500, 500), glm::ivec2(0, 2), 192.0f, 96.0f, 6, 3);
	slime1_->SetScale(6.0f);
}
void Game::Update(float dt)
{

}

void Game::ProcessInput(float dt)
{

}

void Game::Render() {
	float scale = static_cast<float>(Height) / 1080.0f; // or Width / 1920.0f
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Width),
									  static_cast<float>(Height), 0.0f);
	if (dog_)
	{dog_->Draw(projection, scale);}

	if (slime1_)
		slime1_->Draw(projection, scale);
}
void Game::SetSize(unsigned int width, unsigned int height) {
	this->Width = width;
	this->Height = height;
}
