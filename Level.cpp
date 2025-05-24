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

	// 🔼 Load level JSON
	std::string path = "resources/levels/level" + std::to_string(index) + ".json";
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to load " << path << "\n";
		return;
	}
	json data;
	file >> data;

	// ✅ Load all shaders
	for (auto& [name, shaderInfo] : data["resources"]["shaders"].items()) {
		ResourceManager::LoadShader(
			shaderInfo["vert"].get<std::string>().c_str(),
			shaderInfo["frag"].get<std::string>().c_str(),
			nullptr,
			name.c_str()
		);
	}

	// ✅ Load all textures
	for (auto& [name, path] : data["resources"]["textures"].items()) {
		ResourceManager::LoadTexture(path.get<std::string>().c_str(), true, name.c_str());
	}

	// ✅ Use loaded shader + tilemap texture
	Shader shader = ResourceManager::GetShader("sprite");
	Texture2D tileTex = ResourceManager::GetTexture("tilemap");

	// ✅ TileMap setup
	int tileWidth = data["tileSize"].value("width", 16);
	int tileHeight = data["tileSize"].value("height", 16);
	int mapWidth = data["levelSize"].value("width", 224);
	int mapHeight = data["levelSize"].value("height", 240);
	tileMap = std::make_unique<TileMap>(shader, tileTex, mapWidth, mapHeight, tileWidth, tileHeight);

	// ✅ Text renderer from JSON
	TextRenderer* textRenderer = new TextRenderer(width, height);
	std::string fontPath = data["font"].value("path", "resources/fonts/OCRAEXT.TTF");
	int fontSize = data["font"].value("size", 15);
	textRenderer->Load(fontPath, fontSize);
	tileMap->SetTextRenderer(textRenderer);

	// ✅ Load tilemap and solid tiles
	tileMap->Load(data["tilemap"]);
	for (int tileID : data["solid"]) solidTiles.insert(tileID);

	// ✅ Player
	auto& playerData = data["player"];
	float px = playerData["x"];
	float py = playerData["y"];
	float pscale = playerData.value("scale", 0.6f);
	Texture2D dogTexture = ResourceManager::GetTexture("dog");
	dog_ = new Dog(shader, dogTexture, glm::vec2(px, py), glm::ivec2(1, 0));
	dog_->SetScale(pscale);

	// ✅ Enemies
	for (const auto& e : data["enemies"]) {
		std::string type = e["type"];
		Shader shader = ResourceManager::GetShader(e.value("shader", "sprite"));
		Texture2D& texture = ResourceManager::GetTexture(e["texture"]);

		glm::vec2 pos = {e["x"], e["y"]};
		glm::ivec2 frame = {e["frameX"], e["frameY"]};
		float fw = e["frameW"];
		float fh = e["frameH"];
		int cols = e["frameCount"];
		int rows = e["animSpeed"];
		float scale = e.value("scale", 1.0f);

		auto enemy = EnemyRegistry::Create(type, shader, texture, pos, frame, fw, fh, cols, rows);
		if (enemy) {
			enemy->SetScale(scale);
			enemies.push_back(std::move(enemy));
		} else {
			std::cerr << "❌ Unknown enemy type: " << type << "\n";
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
