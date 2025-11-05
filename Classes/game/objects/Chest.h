#pragma once
#include "game/Enemy.h"

class Chest : public Enemy {
public:
    CREATE_FUNC(Chest);

    bool init() override;
    void enablePhysicsTop(const cocos2d::Vec2& topCenter, const cocos2d::Size& sz);

    void takeHit(int dmg) override;   // bị chém/bắn
    int  hp = 3;

private:
    void spawnLootAndVanish();
};
