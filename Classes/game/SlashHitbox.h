#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

// Hitbox cảm biến tồn tại rất ngắn, dùng cho đòn chém K
class SlashHitbox : public cocos2d::Node {
public:
    static SlashHitbox* create(const cocos2d::Size& box, float life = 0.12f);
    bool initWith(const cocos2d::Size& box, float life);

private:
    float _life = 0.12f;
    void _tick(float dt);
};
