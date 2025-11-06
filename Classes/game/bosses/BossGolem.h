#pragma once
#include "game/Enemy.h"
class Player;

class BossGolem : public Enemy {
public:
    static BossGolem* create();
    bool init() override;
    void takeHit(int dmg) override;              // rơi Star khi chết

private:
    void _tickBoss(float dt);
    void _blinkStrike(const cocos2d::Vec2& playerPos); // dịch chuyển + AOE
    void _radialBurst(const cocos2d::Vec2& playerPos); // vòng đạn + vài viên homing

    float _cd = 1.0f;

    // đảm bảo chỉ rơi Star đúng 1 lần (khung hình cuối)
    bool _starDropped = false;
};
