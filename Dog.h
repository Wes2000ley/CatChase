#ifndef DOG_H
#define DOG_H

#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"
#include "TileMap.h"

class Dog {
public:
	Dog(Shader& shader, Texture2D& texture, glm::vec2 position, glm::ivec2 frame);
	void Draw(const glm::mat4& projection, float scale);
	void SetScale(float manscale);


	void Update(float dt, TileMap* tileMap);

	void SetVelocity(const glm::vec2& vel) { velocity_ = vel; }


private:
	Shader shader_;
	Texture2D texture_;
	glm::vec2 position_;
	glm::ivec2 frame_;
	float manscale_ = 1.0f;
	glm::vec2 velocity_ = glm::vec2(0.0f);
	float speed_ = 150.0f;

	static unsigned int quadVAO_;
	void initRenderData();
};



#endif
