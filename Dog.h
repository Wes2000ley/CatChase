#ifndef DOG_H
#define DOG_H

#include <unordered_set>
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"
#include "TileMap.h"

#include <memory>

#include "Collision.h"

enum class Direction8 {
	Right = 0,
	DownRight = 1,
	Down = 2,
	DownLeft = 3,
	Left = 4,
	UpLeft = 5,
	Up = 6,
	UpRight = 7
};


class Dog {
public:
	Dog(std::shared_ptr<Shader> shader,
		std::shared_ptr<Texture2D> texture,
		glm::vec2 position,
		glm::ivec2 frame);

	void Draw(const glm::mat4& projection);

	Circle ComputeBoundingCircle() const;
	float GetScale() const { return manscale_; }

	void SetScale(float manscale);
	void Update(
	float dt,
	const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
	const std::unordered_set<int>& solidTiles,
	int tileWidth,
	int tileHeight,
	glm::vec2 screenSize);

	glm::vec2 GetPosition() const;
	void SetPosition(const glm::vec2& pos);
	void SetVelocity(glm::vec2 v);


private:
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> texture_;

	glm::vec2 position_;
	glm::ivec2 frame_;
	float manscale_ = 1.0f;
	glm::vec2 velocity_ = glm::vec2(0.0f);
	float speed_ = 150.0f;
	Direction8 facingDirection_ = Direction8::Down;
	Circle boundingCircle_;




	static unsigned int quadVAO_;
	static unsigned int quadVBO_;
	void initRenderData();
};




#endif
