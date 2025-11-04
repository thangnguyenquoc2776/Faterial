#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"
#include "game/Entity.h"

class Player;

class Enemy : public Entity {
public:
    CREATE_FUNC(Enemy);

    bool init() override;

    // Tạo body và đặt vị trí
    void enablePhysics(const cocos2d::Vec2& pos,
                       const cocos2d::Size& bodySize = cocos2d::Size(42, 42));

    // AI cơ bản: tuần tra qua lại giữa 2 điểm
    void setPatrol(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    void setTarget(Player* p) { _target = p; }

    // Bị trúng đòn
    virtual void takeHit(int dmg);

    // Tick đơn giản cho AI
    void updateEnemy(float dt);

    // Tinh chỉnh
    void setMoveSpeed(float v) { _moveSpeed = v; }
    void setMaxHp(int v)       { _maxHp = _hp = std::max(1, v); }

protected:
    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& sz);
    void applyEnemyMasks();

protected:
    cocos2d::Sprite*      _sprite  = nullptr;
    cocos2d::PhysicsBody* _body    = nullptr;
    Player*               _target  = nullptr;

    cocos2d::Vec2 _pA, _pB;
    int           _dir = +1; // hướng tuần tra
    float         _moveSpeed = 90.f;

    int _hp    = 2;
    int _maxHp = 2;
};
