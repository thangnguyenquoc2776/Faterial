#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"
#include "game/Entity.h"

class Player : public Entity {
public:
    static Player* create(const std::string& frame = "");
    bool initWithFrame(const std::string& frame);

    // pos = CENTER (tọa độ tâm collider)
    void enablePhysics(const cocos2d::Vec2& pos,
                       const cocos2d::Size& bodySize = cocos2d::Size::ZERO);

    // điều khiển
    void setMoveDir(const cocos2d::Vec2& dir);
    void jump();
    void incFoot(int delta);

    // combat
    void hurt(int dmg, const cocos2d::Vec2& knockImpulse = cocos2d::Vec2::ZERO);

    // tick
    void updateMove(float dt);

    // trạng thái
    int  facing() const     { return _facing; }        // -1 / +1
    bool invincible() const { return _invincible; }

    // kích thước collider
    cocos2d::Size colliderSize() const { return _boxSize; }
    float halfH() const                { return _boxSize.height * 0.5f; }

    // params
    int   hp        = 3;
    int   maxHp     = 3;
    float moveSpeed = 180.0f;
    float jumpImpulse = 380.0f;
    float airControl  = 0.6f;

protected:
    bool init() override { return Entity::init(); }

private:
    cocos2d::Sprite*      _sprite = nullptr;
    cocos2d::PhysicsBody* _body   = nullptr;
    cocos2d::Vec2         _moveDir = cocos2d::Vec2::ZERO;

    int   _footContacts = 0;
    int   _facing       = +1;
    bool  _invincible   = false;

    cocos2d::Size        _boxSize{32,48};

    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& wantSize);
    void applyPlayerMasks();
    void applyVelocity(float dt);
};
