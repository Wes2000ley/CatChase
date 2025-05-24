//
// Created by Skyri on 5/22/2025.
//

#ifndef ENEMIES_H
#define ENEMIES_H
#include "Enemy.h"
#include "EnemyRegistry.h"
#include <iostream>
#include <GLFW/glfw3.h> // ✅ Add this


class SlimeEnemy : public Enemy {
public:
    using Enemy::Enemy;

void Update(float dt,
            const std::vector<std::unique_ptr<TileMap>>& layers,
            const std::unordered_set<int>& solidTiles,
            const glm::vec4& playerBounds) override
{
    // 🔁 Animate Idle
    animTimer_ += dt;
    if (animTimer_ >= animSpeed_) {
        animTimer_ = 0.0f;
        currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
        SetFrame(glm::ivec2(currentFrame_, idleRow_));
    }

    // 🤖 Patrol Logic
    patrolTimer_ += dt;
    if (patrolTimer_ >= patrolInterval_ || patrolDirection_ == glm::vec2(0.0f)) {
        patrolTimer_ = 0.0f;
        int dx = (rand() % 3) - 1;
        patrolDirection_ = glm::vec2(dx, 0.0f); // Only horizontal
    }

    velocity_ = patrolDirection_ * 20.0f;
    glm::vec2 newPos = GetPosition() + velocity_ * dt;

    float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
    float frameHeight = (sheetHeight_ / frameRows_) * manscale_;
    glm::vec2 topLeft     = newPos;
    glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

    if (layers.empty() || layers[0]->GetMapData().empty())
        return;

    int tileWidth  = layers[0]->GetTileWidth();
    int tileHeight = layers[0]->GetTileHeight();
    int mapWidth   = layers[0]->GetMapData()[0].size() * tileWidth;
    int mapHeight  = layers[0]->GetMapData().size() * tileHeight;

    // 🚧 Prevent going out of bounds
    if (newPos.x < 0 || newPos.y < 0 || bottomRight.x > mapWidth || bottomRight.y > mapHeight) {
        patrolDirection_ = glm::vec2(0.0f);
        velocity_ = glm::vec2(0.0f);
        return;
    }

    // ✅ Tile collision
    int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
    int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
    int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
    int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

    bool collided = false;
    for (int y = tileY1; y <= tileY2; ++y) {
        for (int x = tileX1; x <= tileX2; ++x) {
            for (size_t i = 0; i < layers.size(); ++i) {
                const auto& mapData = layers[i]->GetMapData();
                if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                    x < 0 || x >= static_cast<int>(mapData[0].size()))
                    continue;

                int tileID = mapData[y][x];
                if (solidTiles.count(tileID)) {
                    collided = true;
                    break;
                }
            }
        }
    }

    if (!collided) {
        SetPosition(newPos);
    } else {
        patrolDirection_ = glm::vec2(0.0f);
        velocity_ = glm::vec2(0.0f);
    }

    // 💥 Player Collision
    glm::vec4 enemyBox = GetBoundingBox();
    bool hitPlayer = (
        playerBounds.x < enemyBox.z && playerBounds.z > enemyBox.x &&
        playerBounds.y < enemyBox.w && playerBounds.w > enemyBox.y
    );

    if (hitPlayer) {
        std::cout << "💥 Slime collided with player!\n";
        // Optionally trigger damage, pushback, etc.
    }
}




    void Attack() override {
        // add behavior later
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = .3f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 4;
    int idleRow_ = 2;

    // Patrol AI
    float patrolTimer_ = 0.0f;
    float patrolInterval_ = 2.0f; // seconds between direction changes
    glm::vec2 patrolDirection_ = glm::vec2(0.0f);
};



class SkeletonEnemy : public Enemy {
public:
    using Enemy::Enemy;

    void Update(float dt,
            const std::vector<std::unique_ptr<TileMap>>& layers,
            const std::unordered_set<int>& solidTiles,
            const glm::vec4& playerBounds) override
{
    // 🔁 Animate Idle
    animTimer_ += dt;
    if (animTimer_ >= animSpeed_) {
        animTimer_ = 0.0f;
        currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
        SetFrame(glm::ivec2(currentFrame_, idleRow_));
    }

    // 🤖 Patrol Logic
    patrolTimer_ += dt;
    if (patrolTimer_ >= patrolInterval_ || patrolDirection_ == glm::vec2(0.0f)) {
        patrolTimer_ = 0.0f;
        int dx = (rand() % 3) - 1;
        patrolDirection_ = glm::vec2(dx, 0.0f); // Only horizontal
    }

    velocity_ = patrolDirection_ * 20.0f;
    glm::vec2 newPos = GetPosition() + velocity_ * dt;

    float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
    float frameHeight = (sheetHeight_ / frameRows_) * manscale_;
    glm::vec2 topLeft     = newPos;
    glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

    if (layers.empty() || layers[0]->GetMapData().empty())
        return;

    int tileWidth  = layers[0]->GetTileWidth();
    int tileHeight = layers[0]->GetTileHeight();
    int mapWidth   = layers[0]->GetMapData()[0].size() * tileWidth;
    int mapHeight  = layers[0]->GetMapData().size() * tileHeight;

    // 🚧 Prevent going out of bounds
    if (newPos.x < 0 || newPos.y < 0 || bottomRight.x > mapWidth || bottomRight.y > mapHeight) {
        patrolDirection_ = glm::vec2(0.0f);
        velocity_ = glm::vec2(0.0f);
        return;
    }

    // ✅ Tile collision
    int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
    int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
    int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
    int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

    bool collided = false;
    for (int y = tileY1; y <= tileY2; ++y) {
        for (int x = tileX1; x <= tileX2; ++x) {
            for (size_t i = 0; i < layers.size(); ++i) {
                const auto& mapData = layers[i]->GetMapData();
                if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                    x < 0 || x >= static_cast<int>(mapData[0].size()))
                    continue;

                int tileID = mapData[y][x];
                if (solidTiles.count(tileID)) {
                    collided = true;
                    break;
                }
            }
        }
    }

    if (!collided) {
        SetPosition(newPos);
    } else {
        patrolDirection_ = glm::vec2(0.0f);
        velocity_ = glm::vec2(0.0f);
    }

    // 💥 Player Collision
    glm::vec4 enemyBox = GetBoundingBox();
    bool hitPlayer = (
        playerBounds.x < enemyBox.z && playerBounds.z > enemyBox.x &&
        playerBounds.y < enemyBox.w && playerBounds.w > enemyBox.y
    );

    if (hitPlayer) {
        std::cout << "💥 Skeleton collided with player!\n";
        // Optionally trigger damage, pushback, etc.
    }
}



    void Attack() override {
        // add behavior later
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = 0.3f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 6;
    int idleRow_ = 9;

    // Patrol AI
    float patrolTimer_ = 0.0f;
    float patrolInterval_ = 2.0f; // seconds between direction changes
    glm::vec2 patrolDirection_ = glm::vec2(0.0f);
};




#endif //ENEMIES_H
