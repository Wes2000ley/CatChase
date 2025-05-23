// Level.cpp
#include "Level.h"

#include <GLFW/glfw3.h>

#include "Dog.h"
#include "RESOURCE_MANAGER.h"
#include "Enemies.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;



Level::Level(): dog_(nullptr) {
}

Level::~Level() { Unload(); }

void Level::Unload() {
	tileMap.reset();
	enemies.clear();
	solidTiles.clear();
}

void Level::Load(int index, unsigned int width, unsigned int height) {

	Unload(); // clean up previous level
	projection_ = glm::ortho(0.0f, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0.0f);

	// Load shader and texture assets
	ResourceManager::LoadShader("resources/shaders/sprite.vert", "resources/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadTexture("resources/textures/48DogSpriteSheet.png", true, "dog");
	ResourceManager::LoadTexture("resources/textures/Slime.png", true, "slime");
	ResourceManager::LoadTexture("resources/textures/Skeleton.png", true, "skeleton");
	ResourceManager::LoadTexture("resources/textures/DesertTilemap16x16.png", true, "TileMap");
	ResourceManager::LoadShader("resources/shaders/line.vert", "resources/shaders/line.frag", nullptr, "grid");
	Shader shader = ResourceManager::GetShader("sprite");
	Texture2D tileTex = ResourceManager::GetTexture("TileMap");

	tileMap = std::make_unique<TileMap>(shader, tileTex, 224, 240, 16, 16);
	// Tile map
TextRenderer* textRenderer = new TextRenderer(width, height);
	textRenderer->Load("resources/fonts/OCRAEXT.TTF", 15);  // Or any valid .ttf

	// Pass the renderer to TileMap
	tileMap->SetTextRenderer(textRenderer);
	// Set your level layout data per index (use JSON or hardcoded)
	std::string path = "resources/levels/level" + std::to_string(index) + ".json";
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to load " << path << "\n";
		return;
	}
	json data;
	file >> data;

	// Load tilemap
	std::vector<std::vector<int>> tileData = data["tilemap"];
	tileMap->Load(tileData);

	// Load solid tiles
	for (int tileID : data["solid"]) {
		solidTiles.insert(tileID);
	}

	// Load player
	float px = data["player"]["x"];
	float py = data["player"]["y"];
	Texture2D dogTexture = ResourceManager::GetTexture("dog");
	dog_ = new Dog(shader, dogTexture, glm::vec2(px, py), glm::ivec2(1, 0));
	dog_->SetScale(0.5f);

	// Load enemies
	for (auto& enemyData : data["enemies"]) {
		std::string type = enemyData["type"];
		float x = enemyData["x"];
		float y = enemyData["y"];
		int fx = enemyData["frameX"];
		int fy = enemyData["frameY"];
		float fw = enemyData["frameW"];
		float fh = enemyData["frameH"];
		int frames = enemyData["frameCount"];
		int speed = enemyData["animSpeed"];

		if (type == "slime") {
			enemies.push_back(std::make_unique<SlimeEnemy>(
				shader, ResourceManager::GetTexture("slime"),
				glm::vec2(x, y), glm::ivec2(fx, fy), fw, fh, frames, speed));
		} else if (type == "skeleton") {
			enemies.push_back(std::make_unique<SkeletonEnemy>(
				shader, ResourceManager::GetTexture("skeleton"),
				glm::vec2(x, y), glm::ivec2(fx, fy), fw, fh, frames, speed));
		}
	}



	}


void Level::Update(float dt) {
	for (auto& enemy : enemies)
		enemy->Update(dt, tileMap.get());
dog_->Update(dt, tileMap.get(), solidTiles);
}

void Level::Render(const glm::mat4& proj) {
	if (tileMap) {
		tileMap->Draw(proj);
		tileMap->DrawDebugGrid(proj);
	}
	for (auto& enemy : enemies)
		enemy->Draw(proj);
	dog_->Draw(proj);
}
void Level::ProcessInput(float dt, const bool* keys) {
	glm::vec2 velocity(0.0f);
	if (keys[GLFW_KEY_W]) velocity.y -= 1.0f;
	if (keys[GLFW_KEY_S]) velocity.y += 1.0f;
	if (keys[GLFW_KEY_A]) velocity.x -= 1.0f;
	if (keys[GLFW_KEY_D]) velocity.x += 1.0f;

	if (glm::length(velocity) > 0.0f)
		velocity = glm::normalize(velocity);

	if (dog_)
		dog_->SetVelocity(velocity * 100.0f); // Adjust speed as needed
}
