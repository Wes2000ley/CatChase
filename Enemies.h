//
// Created by Skyri on 5/22/2025.
//

#ifndef ENEMIES_H
#define ENEMIES_H
#include "Enemy.h"

class SkeletonEnemy : public Enemy {
public:
	using Enemy::Enemy; // inherit constructor

	void Update(float dt) {
		// walk back and forth
	}

	void Attack() {
		// shoot bone or play animation
	}
};

class SlimeEnemy : public Enemy {
public:
	using Enemy::Enemy; // inherit constructor

	void Update(float dt) {
		// walk back and forth
	}

	void Attack() {
		//test
	}
};




#endif //ENEMIES_H
