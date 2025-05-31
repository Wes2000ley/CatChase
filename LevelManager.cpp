// LevelManager.cpp
#include "LevelManager.h"

#include <iostream>

void LevelManager::LoadLevel(int index, unsigned int width, unsigned int height) {
	lastWindowWidth  = width;
	lastWindowHeight = height;
	UnloadLevel();
	level = std::make_unique<Level>();
	level->Load(index, width, height);
	currentLevelIndex = index;
}


void LevelManager::Update(float dt) {
	if (!level) return;

	// 1) Call Level::Update() and see if it returned a new level index
	int nextLevel = level->Update(dt);

	// 2) If nextLevel >= 0, it means “transition triggered” → load that level
	if (nextLevel >= 0) {
		std::cout << "[LevelManager] Loading level " << nextLevel << "\n";
		LoadLevel(nextLevel, lastWindowWidth , lastWindowHeight);
	}
}

void LevelManager::Render(const glm::mat4& proj) {
	if (level) level->Render(proj);
}

void LevelManager::UnloadLevel() {
	if (level) level->Unload();
	level.reset();
}
void LevelManager::ProcessInput(float dt, const bool* keys) {
	if (level)
		level->ProcessInput(dt, keys);
}
