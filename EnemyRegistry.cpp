#include "EnemyRegistry.h"
#include "Enemies.h"

// Internal registry storage
std::unordered_map<std::string, EnemyConstructor>& EnemyRegistry::GetRegistry() {
	static std::unordered_map<std::string, EnemyConstructor> registry;
	return registry;
}

// Register enemy type
void EnemyRegistry::Register(const std::string& type, EnemyConstructor ctor) {
	GetRegistry()[type] = std::move(ctor);
}

// Create enemy by type
std::unique_ptr<Enemy> EnemyRegistry::Create(
	const std::string& type,
	std::shared_ptr<Shader> shader,
	std::shared_ptr<Texture2D> texture,
	glm::vec2 pos, glm::ivec2 frame,
	float fw, float fh, int frameCols, int frameRows)
{
	auto it = GetRegistry().find(type);
	if (it != GetRegistry().end()) {
		return it->second(shader, texture, pos, frame, fw, fh, frameCols, frameRows);
	}
	return nullptr;
}

// Automatically register enemy types
REGISTER_ENEMY_TYPE_ALIAS("slime", SlimeEnemy)
REGISTER_ENEMY_TYPE_ALIAS("skeleton", SkeletonEnemy)
