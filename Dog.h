#pragma once
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

class Dog {
public:
	Dog(Shader& shader, Texture2D& texture, glm::vec2 position, glm::ivec2 frame);
	void Draw(const glm::mat4& projection) const;

private:
	Shader& shader_;
	Texture2D& texture_;
	glm::vec2 position_;
	glm::ivec2 frame_;

	static unsigned int quadVAO_;
	static void initRenderData();
};
