#pragma once
#include "game/Enemy.h"

class BossGolem : public Enemy {
public:
    static BossGolem* create();
    bool init() override;
    void takeHit(int dmg) override; // rơi sao khi chết + có thể rơi chest/loot

private:
    float _speed = 0.0f;
};
