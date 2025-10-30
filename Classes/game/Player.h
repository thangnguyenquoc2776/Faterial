#pragma once
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"

class Player : public cocos2d::Sprite {
public:
    static Player* create();
    bool init() override;

    void enablePhysics(const cocos2d::Vec2& pos);
    void setMoveDir(const cocos2d::Vec2& dir);
    void jump();
    void fastFall();
    void update(float dt) override;

    // combat
    void shoot();                 // J
    void slash();                 // K

    // damage / hp
    void takeDamage(int dmg, const cocos2d::Vec2& knock);

    // expose
    cocos2d::PhysicsBody* getBody() const { return _body; }
    int hp() const { return _hp; }
    int hpMax() const { return _hpMax; }
    bool invuln() const { return _iframe > 0.f; }

    // ground sensor API (Scene sẽ gọi khi va chạm chân)
    void footBegin();
    void footEnd();

private:
    cocos2d::PhysicsBody* _body=nullptr;
    cocos2d::Vec2 _moveDir{0.f,0.f};
    float _speed=240.f;

    // ground
    int _footContacts=0;
    bool _onGround=false;

    // hp
    int _hpMax=5;
    int _hp=5;
    float _iframe=0.f;     // giây còn lại được miễn sát thương

    // facing
    int _facing=1;         // -1 trái, +1 phải
};
