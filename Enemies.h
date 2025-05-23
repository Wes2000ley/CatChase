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
	void Update(float dt, TileMap* tileMap){}

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
	void Update(float dt, TileMap* tileMap){}

};




#endif //ENEMIES_H
