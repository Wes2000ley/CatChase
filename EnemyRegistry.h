#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <memory>

#include "Enemy.h"

// Macro for registering enemy types with alias
#define REGISTER_ENEMY_TYPE_ALIAS(alias, ClassName) \
namespace { \
const bool registered_##ClassName = [] { \
EnemyRegistry::Register(alias, [](std::shared_ptr<Shader> s, std::shared_ptr<Texture2D> t, glm::vec2 p, glm::ivec2 f, float fw, float fh, int fc, int fr) { \
return std::make_unique<ClassName>(s, t, p, f, fw, fh, fc, fr); \
}); \
return true; \
}(); \
}

// Constructor type used in registry
using EnemyConstructor = std::function<std::unique_ptr<Enemy>(
	std::shared_ptr<Shader>, std::shared_ptr<Texture2D>,
	glm::vec2, glm::ivec2, float, float, int, int)>;

class EnemyRegistry {
public:
	static void Register(const std::string& type, EnemyConstructor ctor);

	static std::unique_ptr<Enemy> Create(
		const std::string& type,
		std::shared_ptr<Shader> shader,
		std::shared_ptr<Texture2D> texture,
		glm::vec2 pos, glm::ivec2 frame,
		float fw, float fh, int frameCols, int frameRows);

private:
	static std::unordered_map<std::string, EnemyConstructor>& GetRegistry();
};
