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
	for (const auto& name : loadedTextureNames)
		ResourceManager::UnloadTexture(name);
	for (const auto& name : loadedShaderNames)
		ResourceManager::UnloadShader(name);

	loadedTextureNames.clear();
	loadedShaderNames.clear();
	enemies.clear();
	solidTiles.clear();
	tileMap.reset();
	if (dog_) {
		dog_.reset();
	}
	tileLayers.clear();

}
void Level::Load(int index, unsigned int width, unsigned int height) {
    Unload();

    std::string path = "resources/levels/level" + std::to_string(index) + ".json";
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to load " << path << "\n";
        return;
    }

    json data;
    file >> data;

    internalWidth = data.value("internalWidth", 496.0f);
    internalHeight = data.value("internalHeight", 272.0f);
    projection_ = glm::ortho(0.0f, internalWidth, internalHeight, 0.0f);

    // Load shaders
    for (auto& [name, shaderInfo] : data["resources"]["shaders"].items()) {
        ResourceManager::LoadShader(
            shaderInfo["vert"].get<std::string>().c_str(),
            shaderInfo["frag"].get<std::string>().c_str(),
            nullptr,
            name.c_str()
        );
        loadedShaderNames.insert(name);
    }

    // Load textures
    for (auto& [name, path] : data["resources"]["textures"].items()) {
        ResourceManager::LoadTexture(path.get<std::string>().c_str(), true, name);
        loadedTextureNames.insert(name);
    }

    // Shared shader and texture
    auto shader = ResourceManager::GetShader("sprite");
    auto tileTex = ResourceManager::GetTexture("tilemap");
    auto sharedText = ResourceManager::GetTextRendererPtr("default");

    int tileWidth = data["tileSize"].value("width", 16);
    int tileHeight = data["tileSize"].value("height", 16);
    int mapWidth = data["levelSize"].value("width", 224);
    int mapHeight = data["levelSize"].value("height", 240);

    // ✅ Layered tilemaps
    tileLayers.clear();
    for (const auto& layer : data["tileLayers"]) {
        auto tilemap = std::make_unique<TileMap>(shader, tileTex, mapWidth, mapHeight, tileWidth, tileHeight);
        tilemap->SetTextRenderer(sharedText);
        tilemap->Load(layer["tilemap"]);
        tileLayers.push_back(std::move(tilemap));
    }

    // ✅ Solid tiles
    for (int tileID : data["solid"]) {
        solidTiles.insert(tileID);
    }

    // ✅ Player
    const auto& playerData = data["player"];
    float px = playerData["x"];
    float py = playerData["y"];
    float pscale = playerData.value("scale", 0.6f);
    auto dogTex = ResourceManager::GetTexture("dog");
    dog_ = std::make_unique<Dog>(shader, dogTex, glm::vec2(px, py), glm::ivec2(1, 0));
    dog_->SetScale(pscale);

    // ✅ Enemies
    for (const auto& e : data["enemies"]) {
        std::string type = e["type"];
        auto shader = ResourceManager::GetShader(e.value("shader", "sprite"));
        auto texture = ResourceManager::GetTexture(e["texture"]);

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
	std::vector<const std::vector<std::vector<int>>*> mapDataPtrs;
	for (const auto& layer : tileLayers)
		mapDataPtrs.push_back(&layer->GetMapData());

	int tileWidth = tileLayers[0]->GetTileWidth();
	int tileHeight = tileLayers[0]->GetTileHeight();
	glm::vec2 screenSize = { internalWidth, internalHeight };

	// 🌀 Get player's current circle
	Circle playerCircle = dog_->ComputeBoundingCircle();

	// 👾 Update enemies
	for (auto& enemy : enemies) {
		enemy->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, playerCircle);
	}

	// 🐕 Update player
	dog_->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, screenSize);
	playerCircle = dog_->ComputeBoundingCircle();

	// 💥 Resolve player <-> enemy overlap
	for (auto& enemy : enemies) {
		Circle enemyCircle = enemy->ComputeBoundingCircle();
		if (!CircleIntersect(playerCircle, enemyCircle)) continue;

		glm::vec2 pushDir = glm::normalize(playerCircle.center - enemyCircle.center);
		glm::vec2 newCenter = playerCircle.center + pushDir * 2.0f;

		Circle pushed = playerCircle;
		pushed.center = newCenter;

		if (!IsCircleBlocked(pushed, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
			dog_->SetPosition(pushed.center - glm::vec2((256.0f / 16.0f) * 0.5f, (48.0f / 3.0f) * 0.5f) * dog_->GetScale());
		} else {
			dog_->SetVelocity(glm::vec2(0.0f));
		}
	}
}




void Level::Render(const glm::mat4& proj) {
	for (auto& layer : tileLayers) {
		layer->Draw(proj);
		// layer->DrawDebugGrid(proj); // optional
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
