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
                const std::unordered_set<int>& solidTiles) override
    {
        // 🔁 Idle Animation
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }

        // 🚧 Collision & Movement (copied from base Enemy)
        glm::vec2 newPos = GetPosition() + velocity_ * dt;

        float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
        float frameHeight = (sheetHeight_ / frameRows_) * manscale_;

        glm::vec2 topLeft     = newPos;
        glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

        if (layers.empty() || layers[0]->GetMapData().empty())
            return;

        int tileWidth  = layers[0]->GetTileWidth();
        int tileHeight = layers[0]->GetTileHeight();

        int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
        int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
        int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
        int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

        for (int y = tileY1; y <= tileY2; ++y) {
            for (int x = tileX1; x <= tileX2; ++x) {
                for (size_t i = 0; i < layers.size(); ++i) {
                    const auto& mapData = layers[i]->GetMapData();
                    if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                        x < 0 || x >= static_cast<int>(mapData[0].size()))
                        continue;

                    int tileID = mapData[y][x];
                    if (solidTiles.count(tileID)) {
                        std::cout << "💢 Slime COLLISION at (" << x << "," << y << ") ID: " << tileID << " [layer " << i << "]\n";
                        velocity_ = glm::vec2(0.0f);
                        return;
                    }
                }
            }
        }

        SetPosition(newPos); // updates bounding box too
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
};



class SkeletonEnemy : public Enemy {
public:
    using Enemy::Enemy;

    void Update(float dt,
                const std::vector<std::unique_ptr<TileMap>>& layers,
                const std::unordered_set<int>& solidTiles) override
    {
        // 🔁 Idle Animation
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }

        // 🚧 Collision & Movement (same as in Enemy)
        glm::vec2 newPos = GetPosition() + velocity_ * dt;

        float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
        float frameHeight = (sheetHeight_ / frameRows_) * manscale_;

        glm::vec2 topLeft     = newPos;
        glm::vec2 bottomRight = newPos + glm::vec2(frameWidth, frameHeight);

        if (layers.empty() || layers[0]->GetMapData().empty())
            return;

        int tileWidth  = layers[0]->GetTileWidth();
        int tileHeight = layers[0]->GetTileHeight();

        int tileX1 = static_cast<int>(topLeft.x) / tileWidth;
        int tileY1 = static_cast<int>(topLeft.y) / tileHeight;
        int tileX2 = static_cast<int>(bottomRight.x) / tileWidth;
        int tileY2 = static_cast<int>(bottomRight.y) / tileHeight;

        for (int y = tileY1; y <= tileY2; ++y) {
            for (int x = tileX1; x <= tileX2; ++x) {
                for (size_t i = 0; i < layers.size(); ++i) {
                    const auto& mapData = layers[i]->GetMapData();
                    if (y < 0 || y >= static_cast<int>(mapData.size()) ||
                        x < 0 || x >= static_cast<int>(mapData[0].size()))
                        continue;

                    int tileID = mapData[y][x];
                    if (solidTiles.count(tileID)) {
                        std::cout << "💢 Skeleton COLLISION at (" << x << "," << y << ") ID: " << tileID << " [layer " << i << "]\n";
                        velocity_ = glm::vec2(0.0f);
                        return;
                    }
                }
            }
        }

        SetPosition(newPos);
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
};




#endif //ENEMIES_H
