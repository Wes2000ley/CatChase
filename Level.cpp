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
	Unload();
	projection_ = glm::ortho(0.0f, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0.0f);

	// Load shaders and other resources
	ResourceManager::LoadShader("resources/shaders/sprite.vert", "resources/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("resources/shaders/line.vert", "resources/shaders/line.frag", nullptr, "grid");
	ResourceManager::LoadTexture("resources/textures/48DogSpriteSheet.png", true, "dog");
	ResourceManager::LoadTexture("resources/textures/Slime.png", true, "slime");
	ResourceManager::LoadTexture("resources/textures/Skeleton.png", true, "skeleton");
	Shader shader = ResourceManager::GetShader("sprite");

	// 🔼 FIRST read the JSON file
	std::string path = "resources/levels/level" + std::to_string(index) + ".json";
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to load " << path << "\n";
		return;
	}
	json data;
	file >> data;

	// ✅ NOW safe to access the JSON contents
	std::string texturePath = data.value("tilemapTexture", "resources/textures/DesertTilemap16x16.png");
	int tileWidth = data["tileSize"].value("width", 16);
	int tileHeight = data["tileSize"].value("height", 16);
	int mapWidth = data["levelSize"].value("width", 224);
	int mapHeight = data["levelSize"].value("height", 240);

	ResourceManager::LoadTexture(texturePath.c_str(), true, "TileMap");
	Texture2D tileTex = ResourceManager::GetTexture("TileMap");

	tileMap = std::make_unique<TileMap>(shader, tileTex, mapWidth, mapHeight, tileWidth, tileHeight);

	TextRenderer* textRenderer = new TextRenderer(width, height);
	std::string fontPath = data["font"].value("path", "resources/fonts/OCRAEXT.TTF");
	int fontSize = data["font"].value("size", 15);
	textRenderer->Load(fontPath, fontSize);
	tileMap->SetTextRenderer(textRenderer);

	// Load tilemap
	std::vector<std::vector<int>> tileData = data["tilemap"];
	tileMap->Load(tileData);

	// Solid tiles
	for (int tileID : data["solid"]) solidTiles.insert(tileID);

	// Player
	float px = data["player"]["x"];
	float py = data["player"]["y"];
	float pscale = data["player"].value("scale", 0.6f);
	Texture2D dogTexture = ResourceManager::GetTexture("dog");
	dog_ = new Dog(shader, dogTexture, glm::vec2(px, py), glm::ivec2(1, 0));
	dog_->SetScale(pscale);

	// Enemies
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
		float scale = enemyData.value("scale", 1.0f);  // fixed syntax

		std::unique_ptr<Enemy> enemy;
		if (type == "slime") {
			enemy = std::make_unique<SlimeEnemy>(shader, ResourceManager::GetTexture("slime"),
				glm::vec2(x, y), glm::ivec2(fx, fy), fw, fh, frames, speed);
		} else if (type == "skeleton") {
			enemy = std::make_unique<SkeletonEnemy>(shader, ResourceManager::GetTexture("skeleton"),
				glm::vec2(x, y), glm::ivec2(fx, fy), fw, fh, frames, speed);
		}

		if (enemy) {
			enemy->SetScale(scale);
			enemies.push_back(std::move(enemy));
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
		//tileMap->DrawDebugGrid(proj);
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
