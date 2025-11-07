// Goomba.h
#pragma once
#include "game/Enemy.h"
class Goomba : public Enemy {
public:
    static Goomba* create();
    bool init() override;
    void updateEnemy(float dt) override;

};
