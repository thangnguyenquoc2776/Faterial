#pragma once
#include "cocos2d.h"
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"

class Player : public Entity {
public:
    CREATE_FUNC(Player);

    bool init() override;

    // Bật vật lý. feetPos = vị trí đặt BÀN CHÂN. Nếu bodySize rỗng -> dùng mặc định.
    void enablePhysics(const cocos2d::Vec2& feetPos,
                       const cocos2d::Size& bodySize = cocos2d::Size::ZERO);

    // Điều khiển
    void setMoveDir(const cocos2d::Vec2& dir);
    void jump();

    // Gameplay state
    void incFoot(int delta);      // tăng/giảm số contact của sensor FOOT
    void hurt(int dmg);
    bool invincible() const { return _invincibleT > 0.f; }

    // HP
    int  hp()    const { return _hp; }
    int  maxHp() const { return _maxHp; }
    void heal(int v);
    void restoreFullHP();
    bool isDead() const { return _hp <= 0; }

    // Trợ giúp cho GameScene/Weapon
    int   facing()   const { return _facing; }            // -1 (trái) / +1 (phải)
    float halfH()    const { return _colSize.height * 0.5f; }
    cocos2d::Size colliderSize() const { return _colSize; }

protected:
    void update(float dt) override;

private:
    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& bodySize);
    void applyPlayerMasks();
    void refreshVisual();

private:
    cocos2d::DrawNode*    _gfx   = nullptr;
    cocos2d::PhysicsBody* _body  = nullptr;

    cocos2d::Size _colSize{28.f, 44.f};
    cocos2d::Vec2 _moveDir{0,0};
    int    _facing        = +1;
    int    _footContacts  = 0;

    // Chuyển động
    float _moveSpeed   = 180.f;   // px/s
    float _accelGround = 1200.f;  // px/s^2
    float _accelAir    = 650.f;   // px/s^2
    float _jumpImpulse = 420.f;   // lực nhảy (nhân khối lượng)
    float _maxFall     = 1200.f;  // clamp vận tốc rơi

    // HP
    int   _hp    = 100;
    int   _maxHp = 100;

    // Bất tử ngắn sau khi bị thương
    float _invincibleT = 0.f;
};
