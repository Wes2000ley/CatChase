#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include "Enemy.h"

#define REGISTER_ENEMY_TYPE_ALIAS(alias, ClassName) \
namespace { \
const bool registered_##ClassName = [] { \
EnemyRegistry::Register(alias, [](Shader& s, Texture2D& t, glm::vec2 p, glm::ivec2 f, float fw, float fh, int fc, int fr) { \
return std::make_unique<ClassName>(s, t, p, f, fw, fh, fc, fr); \
}); \
return true; \
}(); \
}



using EnemyConstructor = std::function<std::unique_ptr<Enemy>(
	Shader&, Texture2D&, glm::vec2, glm::ivec2, float, float, int, int)>;

class EnemyRegistry {
public:
	static void Register(const std::string& type, EnemyConstructor ctor);
	static std::unique_ptr<Enemy> Create(const std::string& type,
		Shader& shader, Texture2D& texture, glm::vec2 pos, glm::ivec2 frame,
		float fw, float fh, int frameCols, int frameRows);

private:
	static std::unordered_map<std::string, EnemyConstructor>& GetRegistry();
};
