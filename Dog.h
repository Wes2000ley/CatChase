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
	void SetCollisionScale(float scale);

	// Call this to initiate a bite (if not already on cooldown)
	void StartBite();

	// Returns true if the dog is in mid‐bite (during the bite animation window)
	bool IsBitingActive() const { return isBiting_; }

	// Computes a small “bite circle” in front of the dog’s facing direction
	Circle ComputeBiteCircle() const;


private:
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> texture_;

	glm::vec2 position_;
	glm::ivec2 frame_;
	float manscale_ = 1.0f;
	float collisionScale_ = 1.0f;
	glm::vec2 velocity_ = glm::vec2(0.0f);
	float speed_ = 150.0f;
	Direction8 facingDirection_ = Direction8::Down;
	Circle boundingCircle_;




	static unsigned int quadVAO_;
	static unsigned int quadVBO_;
	void initRenderData();

	bool isBiting_       = false;   // true while the bite “animation” is active
	float biteTimer_     = 0.0f;    // how much longer we remain in the “bite” state
	float biteCooldown_  = 0.0f;    // how much longer until we can bite again

	static constexpr float biteDuration_     = 0.1f;  // bite lasts 0.3 seconds
	static constexpr float biteCooldownTime_ = 0.5f;  // after bite ends, wait 0.5s before next

};




#endif
