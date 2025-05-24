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


};

class SkeletonEnemy : public Enemy {
public:
	using Enemy::Enemy;

	void Attack() {
		// placeholder
	}


};



#endif //ENEMIES_H
