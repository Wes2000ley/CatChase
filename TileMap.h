#ifndef TILEMAP_H
#define TILEMAP_H

#include "shader.h"
#include "texture.h"
#include <vector>
#include <glm/glm.hpp>
#include "text_renderer.h"

class TileMap {
public:
	TileMap(std::shared_ptr<Shader> shader,
		std::shared_ptr<Texture2D> tileset,
		int textureWidth, int textureHeight,
		int tileWidth, int tileHeight);


	void Load(const std::vector<std::vector<int>>& mapData);
	void Draw(const glm::mat4 & projection);

	int GetTileWidth() const { return tileWidth_; }
	int GetTileHeight() const { return tileHeight_; }
	const std::vector<std::vector<int>>& GetMapData() const { return mapData_; }
	void DrawDebugGrid(const glm::mat4& projection);
	void SetTextRenderer(TextRenderer* renderer) { textRenderer_ = renderer; }

private:
	std::shared_ptr<Shader> shader_;
	std::shared_ptr<Texture2D> tileset_;
	int textureWidth_{}, textureHeight_{};
	int tileWidth_, tileHeight_;
	std::vector<std::vector<int>> mapData_;
	int tilesPerRow_, tilesPerCol_;
	TextRenderer* textRenderer_ = nullptr;

	static unsigned int quadVAO_;
	void initRenderData() const;

	void initGridLines();
	mutable unsigned int gridVAO_ = 0;
	mutable unsigned int gridVBO_ = 0;
	mutable std::vector<float> gridLines_;
};

#endif
