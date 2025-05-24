#ifndef ENEMY_H
#define ENEMY_H

#include <unordered_set>

#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

#include "TileMap.h"

class Enemy {
public:
	Enemy(std::shared_ptr<Shader> shader, std::shared_ptr<Texture2D> texture,
		  glm::vec2 position, glm::ivec2 frame,
		  float sheetWidth, float sheetHeight,
		  int frameCols, int frameRows);

	virtual ~Enemy(); // Already declared, just make sure it's virtual

	virtual void Draw(const glm::mat4 & projection);
	void SetFrame(glm::ivec2 frame);
	void SetPosition(glm::vec2 position);
	void SetScale(float manscale);
	virtual void Update(float dt, const std::vector<std::unique_ptr<TileMap>> &layers, const std::unordered_set<int> &solidTiles) ;
	glm::vec4 GetBoundingBox() const;
	glm::vec4 ComputeBoundingBox() const;
	virtual void Attack() { /* default = do nothing */ }
	glm::vec2 GetPosition() const { return position_; }


private:
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> texture_;
	glm::vec4 boundingBox_;



	static unsigned int quadVAO_;
	void initRenderData();
	static unsigned int quadVBO_;


protected:

	glm::vec2 position_;
	glm::ivec2 frame_;
	float sheetWidth_, sheetHeight_;
	int frameCols_, frameRows_;
	float manscale_ = 1.0f;
	glm::vec2 velocity_ = glm::vec2(0.0f);

};

#endif
