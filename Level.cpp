// Level.cpp
#include "Level.h"

#include <GLFW/glfw3.h>

#include "Dog.h"
#include "RESOURCE_MANAGER.h"
#include "Enemies.h"
#include <fstream>
#include <iostream>
#include "DebugDraw.h"
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

	currentLevel_ = index;
	transitionCooldown_ = 0.0f;

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
        ResourceManager::LoadTexture(path.get<std::string>().c_str(), name);
        loadedTextureNames.insert(name);
    }

    // Shared shader and texture
    auto shader = ResourceManager::GetShader("sprite");
    auto tileTex = ResourceManager::GetTexture("tilemap");
    auto sharedText = ResourceManager::GetTextRendererPtr("default");
	debugShader_ = ResourceManager::GetShader("grid"); // Or whatever debug shader you use


    int tileWidth = data["tileSize"].value("width", 16);
    int tileHeight = data["tileSize"].value("height", 16);
    int mapWidth = data["levelSize"].value("width", 224);
    int mapHeight = data["levelSize"].value("height", 240);

    // ✅ Layered tilemaps
    tileLayers.clear();
	for (auto it = data["tileLayers"].rbegin(); it != data["tileLayers"].rend(); ++it) {
		const auto& layer = *it;

		auto tilemap = std::make_unique<TileMap>(shader, tileTex, mapWidth, mapHeight, tileWidth, tileHeight);
		tilemap->SetTextRenderer(sharedText);

		if (layer.contains("tilemap") && layer["tilemap"].is_array()) {
			tilemap->Load(layer["tilemap"]);
		} else {
			std::cerr << "Layer missing or invalid tilemap\n";
			continue;
		}

		bool isCollidable = layer.value("collidable", false);
		tilemap->SetCollidable(isCollidable);
		std::cout << "Loaded layer with collidable = " << isCollidable << "\n";

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
	float pcollscale = playerData.value("collisionScale", 1.0f);
    auto dogTex = ResourceManager::GetTexture("dog");
    dog_ = std::make_unique<Dog>(shader, dogTex, glm::vec2(px, py), glm::ivec2(1, 0));
    dog_->SetScale(pscale);
	dog_->SetCollisionScale(pcollscale);


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
    	float collscale = e.value("collisionScale", 1.0f);

        auto enemy = EnemyRegistry::Create(type, shader, texture, pos, frame, fw, fh, cols, rows);
        if (enemy) {
            enemy->SetScale(scale);
        	enemy->SetCollisionScale(collscale);

            enemies.push_back(std::move(enemy));
        } else {
            std::cerr << "❌ Unknown enemy type: " << type << "\n";
        }
    }

	transitions_.clear();
	if (data.contains("transitions")) {
		for (const auto& t : data["transitions"]) {
			LevelTransition lt{};
			lt.pos       = glm::ivec2(t["x"], t["y"]);
			lt.size      = glm::ivec2(t.value("width", 1), t.value("height", 1));
			lt.targetLevel = t["targetLevel"];
			if (t.contains("spawnX") && t.contains("spawnY")) {
				lt.spawn = glm::vec2(t["spawnX"], t["spawnY"]);
			}
			transitions_.push_back(lt);
		}
	}
	// *** DEBUG PRINT ***
	std::cout << "[Level::Load] Loaded " << transitions_.size()
			  << " transitions for level " << index << ":\n";
	for (size_t i = 0; i < transitions_.size(); ++i) {
		const auto& lt = transitions_[i];
		std::cout << "    [" << i << "] pos=(" << lt.pos.x << "," << lt.pos.y
				  << "), size=(" << lt.size.x << "," << lt.size.y
				  << "), targetLevel=" << lt.targetLevel;
		if (lt.spawn.has_value()) {
			std::cout << ", spawn=(" << lt.spawn->x << "," << lt.spawn->y << ")";
		}
		std::cout << "\n";
	}


}




int Level::Update(float dt) {
    // 1) Build a list of pointers to each collidable layer’s map data
    std::vector<const std::vector<std::vector<int>>*> mapDataPtrs;
    for (const auto& layer : tileLayers) {
        if (layer->IsCollidable()) {
            const auto& data = layer->GetMapData();
            if (!data.empty() && !data[0].empty()) {
                mapDataPtrs.push_back(&data);
            }
        }
    }

    // If there's no valid collidable map data, skip everything and return “no transition”
    if (mapDataPtrs.empty()) {
        std::cerr << "❌ No valid collidable map data! Skipping update to avoid crash.\n";
        return -1; // <— MUST return -1 here (meaning “no level change”)
    }

    // 2) Gather tile sizes and screen size
    int tileWidth  = tileLayers[0]->GetTileWidth();
    int tileHeight = tileLayers[0]->GetTileHeight();
    glm::vec2 screenSize = { internalWidth, internalHeight };

    // 3) Update enemies
    Circle playerCircle = dog_->ComputeBoundingCircle();
    for (auto& enemy : enemies) {
        enemy->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, playerCircle);
    }

    // 4) Update the player (dog)
    dog_->Update(dt, mapDataPtrs, solidTiles, tileWidth, tileHeight, screenSize);
    playerCircle = dog_->ComputeBoundingCircle();

    // 5) Resolve any player‐enemy overlap
    for (auto& enemy : enemies) {
        Circle enemyCircle = enemy->ComputeBoundingCircle();
        if (!CircleIntersect(playerCircle, enemyCircle)) continue;

        std::cout << "💥 Player collided with enemy!\n";
        glm::vec2 pushDir = glm::normalize(playerCircle.center - enemyCircle.center);
        glm::vec2 newCenter = playerCircle.center + pushDir * 2.0f;
        Circle pushed = playerCircle;
        pushed.center = newCenter;

        if (!IsCircleBlocked(pushed, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
            // Compute how far to move the dog’s sprite (half‐width/height of the sprite)
            float halfW = ((256.0f / 16.0f) * 0.5f) * dog_->GetScale();
            float halfH = ((48.0f / 3.0f) * 0.5f) * dog_->GetScale();
            dog_->SetPosition(pushed.center - glm::vec2(halfW, halfH));
        } else {
            dog_->SetVelocity({0.0f, 0.0f});
        }
    }

    // 6) Check for level transition (tile‐space)
    glm::vec2 playerCenter = dog_->ComputeBoundingCircle().center;
    int tileX = static_cast<int>(playerCenter.x) / tileWidth;
    int tileY = static_cast<int>(playerCenter.y) / tileHeight;

    if (transitionCooldown_ > 0.0f) {
        transitionCooldown_ -= dt;
    } else {
        for (const auto& t : transitions_) {
            bool insideX = (tileX >= t.pos.x && tileX <  t.pos.x + t.size.x);
            bool insideY = (tileY >= t.pos.y && tileY <  t.pos.y + t.size.y);

            if (insideX && insideY && t.targetLevel != currentLevel_) {
                std::cout << "[Level::Update] 🔁 Transition to level "
                          << t.targetLevel << "\n";
                lastLevel_ = currentLevel_;

                // If there is a spawn override, convert it from tile→pixel now
                glm::vec2 spawnOverride = dog_->GetPosition();
                bool hasSpawn = false;
                if (t.spawn.has_value()) {
                    spawnOverride = glm::vec2(
                        t.spawn->x * tileWidth,
                        t.spawn->y * tileHeight
                    );
                    hasSpawn = true;
                }

                transitionCooldown_ = 1.0f; // one‐second cooldown to avoid immediate retrigger

                // Return the index of the next level; LevelManager will actually load it
                return t.targetLevel;
            }
        }
    }

    // 7) No transition triggered: return -1
    return -1;
}





void Level::Render(const glm::mat4& proj) {
	for (auto& layer : tileLayers)
		layer->Draw(proj);

	for (auto& enemy : enemies)
		enemy->Draw(proj);

	dog_->Draw(proj);

	if (debugMode_ && debugShader_) {
		// 🔴 Debug player + enemy circles
		DrawDebugCircle(dog_->ComputeBoundingCircle(), glm::vec3(1.0f, 0.0f, 0.0f), proj, debugShader_);
		for (auto& enemy : enemies)
			DrawDebugCircle(enemy->ComputeBoundingCircle(), glm::vec3(0.0f, 1.0f, 0.0f), proj, debugShader_);

		// 🟩 Draw debug grid from the layer with most rows
		if (!tileLayers.empty()) {
			TileMap* bestLayer = tileLayers[0].get();
			size_t maxRows = bestLayer->GetMapData().size();
			size_t maxCols = maxRows > 0 ? bestLayer->GetMapData()[0].size() : 0;

			for (const auto& layer : tileLayers) {
				const auto& data = layer->GetMapData();
				size_t rows = data.size();
				size_t cols = rows > 0 ? data[0].size() : 0;
				if ((rows > maxRows) || (rows == maxRows && cols > maxCols)) {
					bestLayer = layer.get();
					maxRows = rows;
					maxCols = cols;
				}
			}

			bestLayer->DrawDebugGrid(proj, debugShader_, solidTiles, tileLayers);
		}
	}
}


void Level::ProcessInput(float dt, const bool* keys) {
	glm::vec2 velocity(0.0f);
	if (keys[GLFW_KEY_W]) velocity.y -= 1.0f;
	if (keys[GLFW_KEY_S]) velocity.y += 1.0f;
	if (keys[GLFW_KEY_A]) velocity.x -= 1.0f;
	if (keys[GLFW_KEY_D]) velocity.x += 1.0f;

static bool tabPressed = false;
	if (keys[GLFW_KEY_TAB]) {
		if (!tabPressed) {
			debugMode_ = !debugMode_;
			tabPressed = true;
		}
	} else {
		tabPressed = false;
	}


	if (glm::length(velocity) > 0.0f)
		velocity = glm::normalize(velocity);

	if (dog_)
		dog_->SetVelocity(velocity * 100.0f); // Adjust speed as needed
}
