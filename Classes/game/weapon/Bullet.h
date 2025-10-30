#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

class Bullet : public cocos2d::Node {
public:
    static Bullet* create(const cocos2d::Vec2& dir, float speed=720.f, float life=1.8f);
    bool initWith(const cocos2d::Vec2& dir, float speed, float life);
    void update(float dt) override;

private:
    cocos2d::PhysicsBody* _body=nullptr;
    cocos2d::Vec2 _dir{1.f,0.f};
    float _speed=720.f;
    float _life=1.8f;
};
