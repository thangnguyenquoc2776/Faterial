#pragma once
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"

class Enemy : public cocos2d::Sprite {
public:
    static Enemy* create();
    bool init() override;

    void setPatrol(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    void setSpeed(float s){ _speed = s; }
    virtual void takeHit(int dmg);
    void enablePhysics(const cocos2d::Vec2& pos, const cocos2d::Size& sz);
    void setTarget(cocos2d::Node* t){ _target=t; }
    int  hp() const { return _hp; }
    int  contactDamage() const { return _contactDmg; }

    void update(float dt) override;

protected:
    cocos2d::PhysicsBody* _body=nullptr;
    cocos2d::Node* _target=nullptr;
    float _speed=80.f;
    int   _hp=3;
    int   _contactDmg=1;
    bool  _right=true;
    cocos2d::Vec2 _pA{0,0}, _pB{200,0};

    float _aggroNear=240.f;
    float _aggroFar =360.f;

    void _stepPatrol(float dt);
};
