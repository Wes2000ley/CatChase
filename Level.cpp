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

inline bool AABBIntersect(const glm::vec4& a, const glm::vec4& b) {
	return a.x < b.z && a.z > b.x && a.y < b.w && a.w > b.y;
}



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
	// 📦 Cache map pointers
	std::vector<const std::vector<std::vector<int>>*> mapDataPtrs;
	for (const auto& layer : tileLayers)
		mapDataPtrs.push_back(&layer->GetMapData());

	int tileWidth = tileLayers[0]->GetTileWidth();
	int tileHeight = tileLayers[0]->GetTileHeight();

	// 🐶 Save old position
	glm::vec2 oldPos = dog_->GetPosition();
	glm::vec4 playerBounds = dog_->GetBoundingBox();
	glm::vec4 playerBox = playerBounds;

	// 👾 Update enemies
	for (auto& enemy : enemies)
		enemy->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, playerBounds);

	// 🐕 Update player movement
	dog_->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, glm::vec2(internalWidth, internalHeight));
	playerBox = dog_->GetBoundingBox();

	// 🧱 Handle collisions
	for (const auto& enemy : enemies) {
		if (!enemy) continue;

		glm::vec4 enemyBox = enemy->GetBoundingBox();
		if (!AABBIntersect(playerBox, enemyBox)) continue;

		int pushes = 0;
		const int maxPushAttempts = 4;

		while (AABBIntersect(playerBox, enemyBox) && pushes++ < maxPushAttempts) {
			glm::vec2 enemyCenter = {(enemyBox.x + enemyBox.z) * 0.5f, (enemyBox.y + enemyBox.w) * 0.5f};
			glm::vec2 dogCenter   = {(playerBox.x + playerBox.z) * 0.5f, (playerBox.y + playerBox.w) * 0.5f};
			glm::vec2 pushDir = dogCenter - enemyCenter;

			if (glm::length(pushDir) == 0.0f) break;
			pushDir = glm::normalize(pushDir);

			float pushStep = 1.0f;
			glm::vec2 newPos = dog_->GetPosition() + pushDir * pushStep;

			float dogW = playerBox.z - playerBox.x;
			float dogH = playerBox.w - playerBox.y;
			newPos.x = std::clamp(newPos.x, 0.0f, internalWidth - dogW);
			newPos.y = std::clamp(newPos.y, 0.0f, internalHeight - dogH);

			glm::vec4 testBox = {newPos.x, newPos.y, newPos.x + dogW, newPos.y + dogH};

			if (!IsBoxBlocked(testBox, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
				dog_->SetPosition(newPos);
				playerBox = testBox;
			} else {
				break; // Wall hit
			}
		}

		dog_->SetVelocity(glm::vec2(0.0f)); // only once per loop
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
