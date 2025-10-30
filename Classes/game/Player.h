#pragma once
#include "game/Entity.h"

class Player : public cocos2d::Sprite {
public:
    static Player* create();
    bool init() override;

    void enablePhysics(const cocos2d::Vec2& pos);
    void setMoveDir(const cocos2d::Vec2& dir);
    void jump();

    // NEW
    void shoot();
    void slash();

    void update(float dt) override;
    bool facingRight() const { return _facingRight; }

private:
    cocos2d::PhysicsBody* _body=nullptr;
    cocos2d::Vec2 _moveDir{0,0};
    float _speed=220.f;
    bool  _onGround=false;
    bool  _facingRight=true;
    float _cdShoot=0.f, _cdSlash=0.f;
    void _syncGroundState();
};
