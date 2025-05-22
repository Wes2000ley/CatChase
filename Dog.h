#ifndef DOG_H
#define DOG_H

#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"

class Dog {
public:
	Dog(Shader& shader, Texture2D& texture, glm::vec2 position, glm::ivec2 frame);
	void Draw(const glm::mat4& projection, float scale);

private:
	Shader shader_;
	Texture2D texture_;
	glm::vec2 position_;
	glm::ivec2 frame_;

	static unsigned int quadVAO_;
	void initRenderData();
};

#endif
