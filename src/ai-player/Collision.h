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
    if (tileWidth <= 0 || tileHeight <= 0 || mapDataPtrs.empty())
        return false;

    // For each collidable layer, clamp the query window to that layer's bounds
    for (const auto* mapData : mapDataPtrs) {
        if (!mapData || mapData->empty()) continue;

        const int rows = static_cast<int>(mapData->size());
        // Some maps can be ragged; derive per-row width later

        // Compute unclamped indices
        int minX = static_cast<int>(std::floor((circle.center.x - circle.radius) / tileWidth));
        int maxX = static_cast<int>(std::floor((circle.center.x + circle.radius) / tileWidth));
        int minY = static_cast<int>(std::floor((circle.center.y - circle.radius) / tileHeight));
        int maxY = static_cast<int>(std::floor((circle.center.y + circle.radius) / tileHeight));

        // Clamp Y range to [0, rows-1]
        minY = std::max(minY, 0);
        maxY = std::min(maxY, rows - 1);
        if (minY > maxY) continue;

        for (int y = minY; y <= maxY; ++y) {
            // Clamp X to this exact row’s width (ragged rows are safe)
            const int rowCols = static_cast<int>((*mapData)[y].size());
            if (rowCols <= 0) continue;

            int clMinX = std::max(minX, 0);
            int clMaxX = std::min(maxX, rowCols - 1);
            if (clMinX > clMaxX) continue;

            for (int x = clMinX; x <= clMaxX; ++x) {
                int tileID = (*mapData)[y][x];
                if (!solidTiles.count(tileID)) continue;

                glm::vec2 tileMin(x * tileWidth,     y * tileHeight);
                glm::vec2 tileMax((x + 1) * tileWidth, (y + 1) * tileHeight);
                glm::vec2 closest = glm::clamp(circle.center, tileMin, tileMax);

                if (glm::distance(circle.center, closest) < circle.radius)
                    return true;
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
    constexpr float kEpsilon = 0.001f;

    if (dt <= 0.0f) return false;

    // total requested move this frame
    glm::vec2 totalMove = velocity * dt;

    // Break movement into small chunks to avoid corner tunneling & stickiness.
    const float chunk = 0.35f * float(std::min(tileWidth, tileHeight)); // ~1/3 tile
    int steps = std::max(1, int(std::ceil(glm::length(totalMove) / chunk)));
    steps = std::min(steps, 8); // cap to keep it cheap

    glm::vec2 subMove = totalMove / float(steps);
    bool movedAny = false;

    auto tryAxis = [&](int axis, glm::vec2 step){
        Circle t = circle;

        if (axis == 0) {
            float lo = t.radius;
            float hi = std::max(lo, bounds.x - t.radius); // never invert range
            t.center.x = glm::clamp(t.center.x + step.x, lo, hi);
        } else {
            float lo = t.radius;
            float hi = std::max(lo, bounds.y - t.radius);
            t.center.y = glm::clamp(t.center.y + step.y, lo, hi);
        }

        if (!IsCircleBlocked(t, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
            circle.center[axis] = t.center[axis];
            return true;
        }
        return false;
    };

    for (int s = 0; s < steps; ++s) {
        glm::vec2 step = subMove;

        // Move on the dominant axis first to encourage sliding along walls.
        bool xFirst = std::abs(step.x) >= std::abs(step.y);
        if (xFirst) {
            if (!tryAxis(0, step)) step.x = 0.0f;
            if (!tryAxis(1, step)) step.y = 0.0f;
        } else {
            if (!tryAxis(1, step)) step.y = 0.0f;
            if (!tryAxis(0, step)) step.x = 0.0f;
        }

        // If still overlapping (corner jam), push out a bit.
        if (IsCircleBlocked(circle, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
            constexpr float pushDist = 1.0f; // a tad larger than before
            static const glm::vec2 dirs[8] = {
                { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
                { 0.7071f, 0.7071f }, { -0.7071f, 0.7071f },
                { 0.7071f, -0.7071f }, { -0.7071f, -0.7071f }
            };
            for (glm::vec2 d : dirs) {
                Circle t = circle; t.center += d * pushDist;
                if (!IsCircleBlocked(t, mapDataPtrs, tileWidth, tileHeight, solidTiles)) {
                    circle.center = t.center;
                    break;
                }
            }
        }

        movedAny |= (step.x != 0.0f || step.y != 0.0f);
    }

    // final safe clamp
    float minX = circle.radius + kEpsilon;
    float maxX = std::max(minX, bounds.x - circle.radius - kEpsilon);
    float minY = circle.radius + kEpsilon;
    float maxY = std::max(minY, bounds.y - circle.radius - kEpsilon);

    circle.center.x = glm::clamp(circle.center.x, minX, maxX);
    circle.center.y = glm::clamp(circle.center.y, minY, maxY);

    // stabilize when basically stopped
    if (!movedAny && glm::length(velocity) < 0.1f) {
        circle.center = glm::round(circle.center * 1000.0f) / 1000.0f;
    }

    return movedAny;
}



// Circle overlap test
inline bool CircleIntersect(const Circle& a, const Circle& b) {
    return glm::distance(a.center, b.center) < (a.radius + b.radius);
}


#endif // COLLISION_H
