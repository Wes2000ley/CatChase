// LevelManager.h
#pragma once
#include <memory>
#include "Level.h"

class LevelManager {
public:
void LoadLevel(int index, unsigned int width, unsigned int height);
	void Update(float dt);
	void Render(const glm::mat4& proj);
	void UnloadLevel();

	Level* GetCurrentLevel() { return level.get(); }
	void ProcessInput(float dt, const bool* keys);


private:
	std::unique_ptr<Level> level;
	int currentLevelIndex = -1;
	unsigned int lastWindowWidth  = 0;
	unsigned int lastWindowHeight = 0;
};
