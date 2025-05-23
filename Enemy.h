#ifndef ENEMY_H
#define ENEMY_H

#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

#include "TileMap.h"

class Enemy {
public:
	Enemy(Shader& shader, Texture2D& texture,
		  glm::vec2 position, glm::ivec2 frame,
		  float sheetWidth, float sheetHeight,
		  int frameCols, int frameRows);

	void Draw(const glm::mat4 & projection);
	void SetFrame(glm::ivec2 frame);
	void SetPosition(glm::vec2 position);
	void SetScale(float manscale);
	virtual void Update(float dt, TileMap* tileMap) = 0;
private:
	Shader shader_;
	Texture2D texture_;
	glm::vec2 position_;
	glm::ivec2 frame_;
	float sheetWidth_, sheetHeight_;
	int frameCols_, frameRows_;
	float manscale_ = 1.0f;

	static unsigned int quadVAO_;
	void initRenderData();
};

#endif
