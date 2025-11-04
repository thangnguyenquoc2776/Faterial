// Classes/game/bosses/BossGolem.h
#pragma once
#include "game/Enemy.h"

class BossGolem : public Enemy {
public:
    static BossGolem* create();
    bool init() override;
    void takeHit(int dmg) override;

private:
    int   _hp    = 0;
    float _speed = 0.0f;  // nếu Enemy đã có cơ chế move riêng, bạn có thể bỏ biến này
};
