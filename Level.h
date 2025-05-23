// Level.h
#pragma once
#include <memory>
#include <vector>
#include <unordered_set>

#include "Dog.h"
#include "TileMap.h"
#include "Enemy.h"

class Level {
public:
	Level();
	~Level();

void Load(int index, unsigned int width, unsigned int height);
                // Load tilemap, enemies, etc.
	void Unload();                          // Free level-specific memory
	void Update(float dt);                 // Update all enemies
	void Render(const glm::mat4& proj);    // Draw tilemap + enemies
	void ProcessInput(float dt, const bool* keys);
	const glm::mat4& GetProjection() const { return projection_; }



	std::unique_ptr<TileMap> tileMap;
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::unordered_set<int> solidTiles;
	Dog* dog_;
	static constexpr float INTERNAL_WIDTH  = 496.0f;
	static constexpr float INTERNAL_HEIGHT = 272.0f;
	glm::mat4 projection_;

};
