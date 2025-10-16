#ifndef ENEMIES_H
#define ENEMIES_H

#include "Enemy.h"
#include "EnemyRegistry.h"
#include "../ai-player/Collision.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include "EnemyAI.h"

// ────────────────────────────────────────────────
// 🟢 Slime Enemy — simple chaser with animation
// ────────────────────────────────────────────────
class SlimeEnemy : public Enemy {
public:
    using Enemy::Enemy;
    EnemyAI ai_;

    void Update(float dt,
                const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
                const std::unordered_set<int>& solidTiles,
                int tileWidth, int tileHeight,
                const Circle& playerCircle) override
    {
        // Update AI movement
        ai_.Update(this, playerCircle, mapDataPtrs, solidTiles, tileWidth, tileHeight, dt);

        // Idle / movement animation
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }
    }

    void Draw(const glm::mat4& projection) override {
        Enemy::Draw(projection);
    }

    void Attack() override {
        // TODO: Implement slime melee attack behavior
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = 0.25f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 4;
    int idleRow_ = 2; // animation row for idle
};

// ────────────────────────────────────────────────
// ⚔️ Skeleton Enemy — smarter chaser / ranged later
// ────────────────────────────────────────────────
class SkeletonEnemy : public Enemy {
public:
    using Enemy::Enemy;
    EnemyAI ai_;

    void Update(float dt,
                const std::vector<const std::vector<std::vector<int>>*>& mapDataPtrs,
                const std::unordered_set<int>& solidTiles,
                int tileWidth, int tileHeight,
                const Circle& playerCircle) override
    {
        // Update AI movement
        ai_.Update(this, playerCircle, mapDataPtrs, solidTiles, tileWidth, tileHeight, dt);

        // Idle / movement animation
        animTimer_ += dt;
        if (animTimer_ >= animSpeed_) {
            animTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % idleFrameCount_;
            SetFrame(glm::ivec2(currentFrame_, idleRow_));
        }
    }

    void Draw(const glm::mat4& projection) override {
        Enemy::Draw(projection);
    }

    void Attack() override {
        // TODO: Implement skeleton ranged or melee attack
    }

private:
    float animTimer_ = 0.0f;
    float animSpeed_ = 0.25f;
    int currentFrame_ = 0;
    int idleFrameCount_ = 6;
    int idleRow_ = 9; // animation row for idle
};

#endif // ENEMIES_H
