#ifndef DOG_H
#define DOG_H

#include <unordered_set>
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"
#include "TileMap.h"

#include <memory>

class Dog {
public:
	Dog(std::shared_ptr<Shader> shader,
		std::shared_ptr<Texture2D> texture,
		glm::vec2 position,
		glm::ivec2 frame);

	void Draw(const glm::mat4& projection);
	void SetScale(float manscale);
	void Update(float dt,
					 const std::vector<std::unique_ptr<TileMap>>& layers,
					 const std::unordered_set<int>& solidTiles,
					 glm::vec2 screenSize);
	void SetVelocity(const glm::vec2& vel) { velocity_ = vel; }

private:
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> texture_;

	glm::vec2 position_;
	glm::ivec2 frame_;
	float manscale_ = 1.0f;
	glm::vec2 velocity_ = glm::vec2(0.0f);
	float speed_ = 150.0f;

	static unsigned int quadVAO_;
	static unsigned int quadVBO_;
	void initRenderData();
};




#endif
