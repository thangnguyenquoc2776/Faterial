// BossGolem.h
#pragma once
#include "game/Enemy.h"
class BossGolem : public Enemy {
public:
    static BossGolem* create();
    bool init() override;
    void takeHit(int dmg) override; // trụ hơn
};
