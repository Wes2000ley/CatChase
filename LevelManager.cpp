// LevelManager.cpp
#include "LevelManager.h"

void LevelManager::LoadLevel(int index, unsigned int width, unsigned int height) {
	UnloadLevel();
	level = std::make_unique<Level>();
	level->Load(index, width, height);
	currentLevelIndex = index;
}


void LevelManager::Update(float dt) {
	if (level) level->Update(dt);
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
