#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

class Bullet : public cocos2d::Node {
public:
    static Bullet* create(const cocos2d::Vec2& dir, float speed = 700.f, float life = 1.6f);
    bool initWith(const cocos2d::Vec2& dir, float speed, float life);

    cocos2d::PhysicsBody* body() const { return _body; }
private:
    cocos2d::PhysicsBody* _body = nullptr;
    cocos2d::Vec2 _dir{1,0};
    float _speed = 700.f;
    float _life  = 1.6f;
    void _tick(float dt);
};
