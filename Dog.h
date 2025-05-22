#ifndef DOG_H
#define DOG_H

#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"

class Dog {
public:
	Dog(Shader& shader, Texture2D& texture, glm::vec2 position, glm::ivec2 frame);
	void Draw(const glm::mat4& projection, float scale);
	void SetScale(float manscale);

private:
	Shader shader_;
	Texture2D texture_;
	glm::vec2 position_;
	glm::ivec2 frame_;
	float manscale_ = 1.0f;

	static unsigned int quadVAO_;
	void initRenderData();
};



#endif
