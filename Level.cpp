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
    // Save previous player position before moving
    glm::vec2 oldPos = dog_->GetPosition();

    // Update enemies and pass stale (initial) bounds for pre-check logic
    glm::vec4 initialBounds = dog_->GetBoundingBox();
    for (auto& enemy : enemies)
        enemy->Update(dt, tileLayers, solidTiles, initialBounds);

    // Update player movement
    dog_->Update(dt, tileLayers, solidTiles, glm::vec2(internalWidth, internalHeight));

    // Get up-to-date player bounding box (after movement)
    glm::vec4 playerBounds = dog_->GetBoundingBox();

    // Check collisions after updates
    for (const auto& enemy : enemies) {
        if (!enemy) continue;

        glm::vec4 enemyBox = enemy->GetBoundingBox();

        bool collision =
            playerBounds.x < enemyBox.z && playerBounds.z > enemyBox.x &&
            playerBounds.y < enemyBox.w && playerBounds.w > enemyBox.y;

        if (collision) {
            std::cout << "💥 Dog collided with enemy! Trying safe pushback...\n";

            glm::vec2 enemyCenter = {
                (enemyBox.x + enemyBox.z) / 2.0f,
                (enemyBox.y + enemyBox.w) / 2.0f
            };
            glm::vec2 dogCenter = {
                (playerBounds.x + playerBounds.z) / 2.0f,
                (playerBounds.y + playerBounds.w) / 2.0f
            };

            glm::vec2 pushDir = glm::normalize(dogCenter - enemyCenter);
            float pushStrength = 5.0f;

            glm::vec2 newPos = dog_->GetPosition() + pushDir * pushStrength;

            float dogWidth = playerBounds.z - playerBounds.x;
            float dogHeight = playerBounds.w - playerBounds.y;
            glm::vec2 topLeft = newPos;
            glm::vec2 bottomRight = newPos + glm::vec2(dogWidth, dogHeight);

            bool blocked = false;
            int tileWidth = tileLayers[0]->GetTileWidth();
            int tileHeight = tileLayers[0]->GetTileHeight();

            int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
            int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
            int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
            int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

            for (int y = tileY1; y <= tileY2 && !blocked; ++y) {
                for (int x = tileX1; x <= tileX2 && !blocked; ++x) {
                    for (const auto& layer : tileLayers) {
                        const auto& mapData = layer->GetMapData();
                        if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                            x < 0 || x >= static_cast<int>(mapData[0].size()))
                            continue;

                        if (solidTiles.count(mapData[y][x])) {
                            blocked = true;
                        }
                    }
                }
            }

            if (!blocked) {
                dog_->SetPosition(newPos);
            } else {
                std::cout << "🚫 Pushback canceled: would hit wall.\n";
                dog_->SetPosition(oldPos); // Reset to safe position
            }

            dog_->SetVelocity(glm::vec2(0.0f));
            break; // only resolve one collision per frame
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
