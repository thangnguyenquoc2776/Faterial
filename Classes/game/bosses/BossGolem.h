#pragma once
#include "game/Enemy.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

class Player;

class BossGolem : public Enemy {
public:
    static BossGolem* create();
    bool init() override;

    // Ghi đè để xử lý chết an toàn + rơi Star (không dùng LootTable)
    void takeHit(int dmg) override;

private:
    void _tickBoss(float dt);
    void _blinkStrike(const cocos2d::Vec2& playerPos); // dịch chuyển + AOE
    void _radialBurst(const cocos2d::Vec2& playerPos); // vòng đạn + vài viên homing

    float _cd = 1.0f;   // cooldown nội bộ luân phiên pattern
    bool  _dying = false; // tránh gọi chết nhiều lần trong cùng 1 frame

    cocos2d::Sprite* _sprite = nullptr;
};
