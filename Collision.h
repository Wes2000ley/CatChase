// Collision.h
#ifndef COLLISION_H
#define COLLISION_H

#include <unordered_set>
#include <glm/glm.hpp>
#include <vector>


extern const std::unordered_set<int> solidTiles;

#pragma once
#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>

// Shared collision helper
inline bool IsBoxBlocked(
	const glm::vec4& box,
	const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
	int tileWidth, int tileHeight,
	const std::unordered_set<int>& solidTiles)
{
	int tileX1 = static_cast<int>(box.x) / tileWidth;
	int tileY1 = static_cast<int>(box.y) / tileHeight;
	int tileX2 = static_cast<int>(box.z) / tileWidth;
	int tileY2 = static_cast<int>(box.w) / tileHeight;

	for (int y = tileY1; y <= tileY2; ++y) {
		for (int x = tileX1; x <= tileX2; ++x) {
			for (const auto* mapData : mapDataPtrs) {
				if (!mapData || y < 0 || x < 0 ||
					y >= static_cast<int>(mapData->size()) ||
					x >= static_cast<int>((*mapData)[0].size())) continue;

				int tileID = (*mapData)[y][x];
				if (solidTiles.count(tileID)) {
					return true;
				}
			}
		}
	}
	return false;
}
inline bool TryMove(glm::vec2& pos,
			 const glm::vec2& velocity,
			 float dt,
			 float width,
			 float height,
			 const glm::vec2& bounds,
			 const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
			 const std::unordered_set<int>& solidTiles,
			 int tileWidth,
			 int tileHeight)
{
	glm::vec2 newPos = pos + velocity * dt;

	// ✅ Clamp to bounds (screen or tilemap)
	newPos.x = glm::clamp(newPos.x, 0.0f, bounds.x - width);
	newPos.y = glm::clamp(newPos.y, 0.0f, bounds.y - height);

	glm::vec4 futureBox = {
		newPos.x,
		newPos.y,
		newPos.x + width,
		newPos.y + height
	};

	if (!IsBoxBlocked(futureBox, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
		pos = newPos;
		return true; // success
	}

	return false; // blocked
}

#endif
