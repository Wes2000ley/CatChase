#ifndef TILEMAP_H
#define TILEMAP_H

#include "shader.h"
#include "texture.h"
#include <vector>
#include <glm/glm.hpp>

class TileMap {
public:
	TileMap(Shader& shader, Texture2D& tileset,
			int textureWidth, int textureHeight,
			int tileWidth, int tileHeight);

	void Load(const std::vector<std::vector<int>>& mapData);
	void Draw(const glm::mat4 & projection);

	int GetTileWidth() const { return tileWidth_; }
	int GetTileHeight() const { return tileHeight_; }
	const std::vector<std::vector<int>>& GetMapData() const { return mapData_; }

private:
	Shader shader_;
	Texture2D tileset_;
	int textureWidth_{}, textureHeight_{};
	int tileWidth_, tileHeight_;
	std::vector<std::vector<int>> mapData_;
	int tilesPerRow_, tilesPerCol_;

	static unsigned int quadVAO_;
	void initRenderData() const;
};

#endif
