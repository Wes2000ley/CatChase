#ifndef ENEMIES_H
#define ENEMIES_H

#include "Enemy.h"
#include "EnemyRegistry.h"
#include "Collision.h"
#include <iostream>
#include <GLFW/glfw3.h> // ✅ Required for input if needed

class SlimeEnemy : public Enemy {
public:
    using Enemy::Enemy;

    void Update(float dt,
                    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
                    const std::unordered_set<int>& solidTiles,
                    int tileWidth, int tileHeight,
                    const Circle& playerCircle) override
    {
        // 🔁 Animate Idle
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }

        // 🤖 Patrol AI
        patrolTimer_ += dt;
        if (patrolTimer_ >= patrolInterval_ || patrolDirection_ == glm::vec2(0.0f)) {
            patrolTimer_ = 0.0f;
            int dx = (rand() % 3) - 1;
            patrolDirection_ = glm::vec2(dx, 0.0f);
        }

        velocity_ = patrolDirection_ * 20.0f;

        float w = (sheetWidth_ / frameCols_) * manscale_;
        float h = (sheetHeight_ / frameRows_) * manscale_;
        Circle c = {
            position_ + glm::vec2(w, h) * 0.5f,
            0.5f * glm::length(glm::vec2(w, h)) * collisionScale_
        };

        glm::vec2 mapSize = {
            mapDataPtrs[0]->at(0).size() * tileWidth,
            mapDataPtrs[0]->size() * tileHeight
        };

        if (!TryMoveCircle(c, velocity_, dt, mapSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
            velocity_ = glm::vec2(0.0f);
            patrolDirection_ = glm::vec2(0.0f);
        } else {
            position_ = c.center - glm::vec2(w, h) * 0.5f;
        }

        // 💥 Circular overlap with player
        if (CircleIntersect(c, playerCircle)) {
            std::cout << "💥 Slime collided with player!\n";
        }
    }

    void Attack() override {
        // TODO: Implement
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = 0.3f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 4;
    int idleRow_ = 2;

    float patrolTimer_ = 0.0f;
    float patrolInterval_ = 2.0f;
    glm::vec2 patrolDirection_ = glm::vec2(0.0f);
};


class SkeletonEnemy : public Enemy {
public:
    using Enemy::Enemy;

    void Update(float dt,
                    const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
                    const std::unordered_set<int>& solidTiles,
                    int tileWidth, int tileHeight,
                    const Circle& playerCircle) override
    {
        // 🔁 Animate Idle
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }

        // 🤖 Patrol AI
        patrolTimer_ += dt;
        if (patrolTimer_ >= patrolInterval_ || patrolDirection_ == glm::vec2(0.0f)) {
            patrolTimer_ = 0.0f;
            int dx = (rand() % 3) - 1;
            patrolDirection_ = glm::vec2(dx, 0.0f);
        }

        velocity_ = patrolDirection_ * 20.0f;

        float w = (sheetWidth_ / frameCols_) * manscale_;
        float h = (sheetHeight_ / frameRows_) * manscale_;
        Circle c = {
            position_ + glm::vec2(w, h) * 0.5f,
            0.5f * glm::length(glm::vec2(w, h)) * collisionScale_
        };

        glm::vec2 mapSize = {
            mapDataPtrs[0]->at(0).size() * tileWidth,
            mapDataPtrs[0]->size() * tileHeight
        };

        if (!TryMoveCircle(c, velocity_, dt, mapSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
            velocity_ = glm::vec2(0.0f);
            patrolDirection_ = glm::vec2(0.0f);
        } else {
            position_ = c.center - glm::vec2(w, h) * 0.5f;
        }

        // 💥 Circular overlap with player
        if (CircleIntersect(c, playerCircle)) {
            std::cout << "💥 Skeleton collided with player!\n";
        }
    }

    void Attack() override {
        // TODO: Implement
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = 0.3f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 6;
    int idleRow_ = 9;

    float patrolTimer_ = 0.0f;
    float patrolInterval_ = 2.0f;
    glm::vec2 patrolDirection_ = glm::vec2(0.0f);
};

#endif // ENEMIES_H
