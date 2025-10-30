#pragma once
#include "game/Entity.h"

class Enemy : public cocos2d::Sprite {
public:
    static Enemy* create();
    bool init() override;

    void setPatrol(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    void setSpeed(float s){ _speed = s; }
    virtual void takeHit(int dmg);
    void enablePhysics(const cocos2d::Vec2& pos, const cocos2d::Size& sz);
    void setTarget(cocos2d::Node* t){ _target=t; }
    int  hp() const { return _hp; }          // << public
    void update(float dt) override;

protected:
    cocos2d::PhysicsBody* _body = nullptr;
    cocos2d::Sprite* _gfx = nullptr;
    cocos2d::Node* _target = nullptr;        // << NEW
    float _speed = 80.f;
    int   _hp    = 3;
    bool  _right = true;
    cocos2d::Vec2 _pA{0,0}, _pB{200,0};

    void _stepPatrol(float dt);
};
