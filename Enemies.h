//
// Created by Skyri on 5/22/2025.
//

#ifndef ENEMIES_H
#define ENEMIES_H
#include "Enemy.h"
#include "EnemyRegistry.h"
#include <GLFW/glfw3.h> // ✅ Add this


class SlimeEnemy : public Enemy {
public:
	using Enemy::Enemy;

	void Attack() {
		// placeholder
	}

	void Update(float dt, TileMap* tileMap) override {
		// ✅ Add movement or test logic here
		position_.x += sin(glfwGetTime()) * 10.0f * dt;
	}
};

class SkeletonEnemy : public Enemy {
public:
	using Enemy::Enemy;

	void Attack() {
		// placeholder
	}

	void Update(float dt, TileMap* tileMap) override {
		// ✅ Add movement or test logic here
		position_.x += cos(glfwGetTime()) * 10.0f * dt;
	}
};



#endif //ENEMIES_H
