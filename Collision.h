#ifndef COLLISION_H
#define COLLISION_H

#include <unordered_set>
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

// Make sure this exists
extern const std::unordered_set<int> solidTiles;

struct Circle {
    glm::vec2 center;
    float radius;
};

// ⛔ Check if circle overlaps any solid tile
inline bool IsCircleBlocked(
    const Circle& circle,
    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
    int tileWidth, int tileHeight,
    const std::unordered_set<int>& solidTiles)
{
    int minX = static_cast<int>((circle.center.x - circle.radius) / tileWidth);
    int maxX = static_cast<int>((circle.center.x + circle.radius) / tileWidth);
    int minY = static_cast<int>((circle.center.y - circle.radius) / tileHeight);
    int maxY = static_cast<int>((circle.center.y + circle.radius) / tileHeight);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            for (const auto* mapData : mapDataPtrs) {
                if (!mapData || y < 0 || x < 0 ||
                    y >= static_cast<int>(mapData->size()) ||
                    x >= static_cast<int>((*mapData)[0].size())) continue;

                int tileID = (*mapData)[y][x];
                if (solidTiles.count(tileID)) {
                    // Get tile bounds
                    glm::vec2 tilePos = glm::vec2(x * tileWidth, y * tileHeight);
                    glm::vec2 tileSize = glm::vec2(tileWidth, tileHeight);
                    glm::vec2 closest = glm::clamp(circle.center, tilePos, tilePos + tileSize);

                    float dist = glm::distance(circle.center, closest);
                    if (dist < circle.radius) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// 🚶‍♂️ Circle move + tile blocking
inline bool TryMoveCircle(
    Circle& circle,
    const glm::vec2& velocity,
    float dt,
    glm::vec2 bounds,
    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
    const std::unordered_set<int>& solidTiles,
    int tileWidth, int tileHeight)
{
    Circle moved = circle;
    moved.center += velocity * dt;

    // Clamp to screen/map bounds
    moved.center.x = glm::clamp(moved.center.x, moved.radius, bounds.x - moved.radius);
    moved.center.y = glm::clamp(moved.center.y, moved.radius, bounds.y - moved.radius);

    if (!IsCircleBlocked(moved, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
        circle = moved;
        return true;
    }

    return false;
}

// Circle overlap test
inline bool CircleIntersect(const Circle& a, const Circle& b) {
    return glm::distance(a.center, b.center) < (a.radius + b.radius);
}


#endif // COLLISION_H
