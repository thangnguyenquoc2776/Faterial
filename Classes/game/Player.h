#pragma once
#include "cocos2d.h"
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"
#include <vector>
#include <string>

class Player : public Entity {
public:
    CREATE_FUNC(Player);
    bool init() override;

    // Vật lý
    void enablePhysics(const cocos2d::Vec2& feetPos,
                       const cocos2d::Size& bodySize = cocos2d::Size::ZERO);

    // Điều khiển
    void setMoveDir(const cocos2d::Vec2& dir);
    void jump();
    void doShoot();
    void doSlash();

    // Trạng thái gameplay
    void incFoot(int delta);
    void hurt(int dmg);
    bool invincible() const { return _invincibleT > 0.f; }

    // HP
    int  hp() const { return _hp; }
    int  maxHp() const { return _maxHp; }
    void heal(int v);
    void restoreFullHP();
    bool isDead() const { return _hp <= 0; }

    // Trợ giúp
    int   facing() const { return _facing; } // -1 / +1
    float halfH() const { return _colSize.height * 0.5f; }
    cocos2d::Size colliderSize() const { return _colSize; }

    // Buff/Upgrade
    void applyUpgrade(int type, float durationSec); // map từ Upgrade::Type
    int  atkBonus() const { return _atkBonus; }

    // Debug/anim state
    std::string nextAnim;

    // (giữ public cho tương thích code cũ)
    bool _attacking = false;
    bool _shooting  = false;

protected:
    void update(float dt) override;

private:
    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& bodySize);
    void applyPlayerMasks();
    void refreshVisual();

    // Animation helpers (để Player.cpp gọi)
    void playAnim(const std::string& name, float delay, int frames);
    cocos2d::Vector<cocos2d::SpriteFrame*> buildFrames(const std::string& animName, int frameCount);

private:
    // Visual
    cocos2d::Sprite* _sprite = nullptr;
    std::string _currentAnim;

    // Body/collider
    cocos2d::PhysicsBody* _body = nullptr;
    cocos2d::Size _colSize{28.f, 44.f};
    cocos2d::Vec2 _moveDir{0,0};
    int _facing = +1;
    int _footContacts = 0;

    // Di chuyển
    float _moveSpeed   = 180.f;
    float _accelGround = 1200.f;
    float _accelAir    = 650.f;
    float _jumpImpulse = 550.f;
    // float _jumpImpulse = 600.f;
    float _maxFall     = 1200.f;

    // HP/invuln
    int _hp = 100;
    int _maxHp = 100;
    float _invincibleT = 0.f;

    // Buff core (chỉ khai 1 lần, không trùng!)
    int   _extraBullets = 0;   // bắn thêm N viên
    float _slashRange   = 60.f;
    float _dmgScale     = 1.0f; // nếu muốn dùng dạng nhân
    int   _atkBonus     = 0;    // đang dùng cộng thẳng

    // Double jump
    int _airJumpsMax  = 0;
    int _airJumpsUsed = 0;

    // Buff runtime (đếm ngược & revert)
    struct ActiveBuff { int type; float remain; float factor; int hudId; };
    std::vector<ActiveBuff> _buffs;
    
    bool _wasRunning = false; // để phát hiện lúc bắt đầu/ kết thúc chạy

};
