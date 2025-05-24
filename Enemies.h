//
// Created by Skyri on 5/22/2025.
//

#ifndef ENEMIES_H
#define ENEMIES_H
#include "Enemy.h"
#include "EnemyRegistry.h"
#include <iostream>
#include <GLFW/glfw3.h> // ✅ Add this

#include "Collision.h"


class SlimeEnemy : public Enemy {
public:
    using Enemy::Enemy;

    void Update(float dt,
             const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
             const std::unordered_set<int>& solidTiles,
             int tileWidth, int tileHeight,
             const glm::vec4& playerBounds) override
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

        float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
        float frameHeight = (sheetHeight_ / frameRows_) * manscale_;
        glm::vec2 mapSize = glm::vec2(
            mapDataPtrs[0]->at(0).size() * tileWidth,
            mapDataPtrs[0]->size() * tileHeight
        );

        if (!TryMove(position_, velocity_, dt, frameWidth, frameHeight, mapSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
            velocity_ = glm::vec2(0.0f);
            patrolDirection_ = glm::vec2(0.0f);
        }
        boundingBox_ = ComputeBoundingBox();


        // 💥 Check for player overlap
        glm::vec4 enemyBox = GetBoundingBox();
        if (playerBounds.x < enemyBox.z && playerBounds.z > enemyBox.x &&
            playerBounds.y < enemyBox.w && playerBounds.w > enemyBox.y) {
            std::cout << "💥 Slime collided with player!\n";
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
            const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
            const std::unordered_set<int>& solidTiles,
            int tileWidth, int tileHeight,
            const glm::vec4& playerBounds) override
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

        float frameWidth  = (sheetWidth_ / frameCols_) * manscale_;
        float frameHeight = (sheetHeight_ / frameRows_) * manscale_;
        glm::vec2 mapSize = glm::vec2(
            mapDataPtrs[0]->at(0).size() * tileWidth,
            mapDataPtrs[0]->size() * tileHeight
        );

        if (!TryMove(position_, velocity_, dt, frameWidth, frameHeight, mapSize, mapDataPtrs, solidTiles, tileWidth, tileHeight)) {
            velocity_ = glm::vec2(0.0f);
            patrolDirection_ = glm::vec2(0.0f);
        }
        boundingBox_ = ComputeBoundingBox();

        // 💥 Check for player overlap
        glm::vec4 enemyBox = GetBoundingBox();
        if (playerBounds.x < enemyBox.z && playerBounds.z > enemyBox.x &&
            playerBounds.y < enemyBox.w && playerBounds.w > enemyBox.y) {
            std::cout << "💥 Skeleton collided with player!\n";
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
