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
#include "Enemies.h"
#include "Enemy.h"
#include "RESOURCE_MANAGER.h"
#include "TileMap.h"


Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

void Game::Init() {


	// Load shader and texture for dog
	ResourceManager::LoadShader("resources/shaders/sprite.vert", "resources/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadTexture("resources/textures/48DogSpriteSheet.png", true, "dog");

	// Load slime texture
	ResourceManager::LoadTexture("resources/textures/Slime.png", true, "slime");

	ResourceManager::LoadTexture("resources/textures/Skeleton.png", true, "skeleton");

	ResourceManager::LoadTexture("resources/textures/DesertTilemap16x16.png", true, "TileMap");

	// Setup shader projection
	Shader shader = ResourceManager::GetShader("sprite");
	shader.Use();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.0f, -1.0f, 1.0f);
	shader.SetMatrix4("projection", projection);

	// Create dog
	Texture2D dogTexture = ResourceManager::GetTexture("dog");
	dog_ = new Dog(shader, dogTexture, glm::vec2(100, 100), glm::ivec2(2, 3));
	dog_->SetScale(6.0f);

	// Create slime enemy
	Texture2D slimeTexture = ResourceManager::GetTexture("slime");
	slime1_ = new SlimeEnemy(shader, slimeTexture, glm::vec2(500, 500), glm::ivec2(0, 2), 192.0f, 96.0f, 6, 3);
	slime1_->SetScale(6.0f);

	// Create skeleton enemy
	Texture2D skeletonTexture = ResourceManager::GetTexture("skeleton");
	skeleton1_ = new SkeletonEnemy(shader, skeletonTexture, glm::vec2(400, 700), glm::ivec2(7, 2), 192.0f, 320.0f, 6, 10);
	skeleton1_->SetScale(4.0f);

	// Tile map texture: 256x256, 16x16 tiles → 16x16 = 16 columns, 16 rows
	Texture2D TileMapTexture = ResourceManager::GetTexture("TileMap");
	TileMap_ = new TileMap(shader, TileMapTexture,
						  224, 240, 16, 16);
	TileMap_->Load({
		{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13 },
		{ 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 },
		{ 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 },
		{ 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55 },
		{ 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69 },
		{ 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83 },
		{ 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97 },
		{ 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111 },
		{ 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125 },
		{ 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139 },
		{ 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153 },
		{ 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167 },
		{ 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181 },
		{ 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195 },
		{ 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209 },
	});



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

	if (TileMap_)
		TileMap_->Draw(projection, Width, Height);

	if (dog_)
	{dog_->Draw(projection, scale);}

	if (slime1_)
		slime1_->Draw(projection, scale);

	if (skeleton1_)
		skeleton1_->Draw(projection, scale);


}
void Game::SetSize(unsigned int width, unsigned int height) {
	this->Width = width;
	this->Height = height;
}
