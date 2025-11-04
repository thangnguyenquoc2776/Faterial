#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"
#include "game/Entity.h"

class Player : public Entity {
public:
    // Factory
    static Player* create(const std::string& frame = "");

    // cocos2d lifecycle
    bool init() override;                         // bắt buộc cho cocos2d
    bool initWithFrame(const std::string& frame); // khởi tạo kèm frame

    // Physics (GameScene đang gọi bản này)
    void enablePhysics(const cocos2d::Vec2& pos,
                       const cocos2d::Size& bodySize = cocos2d::Size::ZERO);

    // Điều khiển
    void setMoveDir(const cocos2d::Vec2& dir);  // [-1..1] mỗi trục
    void jump();                                // chỉ nhảy khi đang đứng đất
    void incFoot(int delta);                    // +1 chạm đất, -1 rời đất

    // Combat/HP
    void hurt(int dmg);                         // không knockback
    void hurt(int dmg, const cocos2d::Vec2& knockImpulse);

    // Tick di chuyển
    void updateMove(float dt);

    // Query (GameScene dùng)
    int  facing() const     { return _facing; }      // -1 trái, +1 phải
    bool invincible() const { return _invincible; }

    // Tham số tuning
    int   hp      = 3;
    int   maxHp   = 3;
    float moveSpeed   = 180.f;   // px/s
    float jumpImpulse = 380.f;   // lực nhảy
    float airControl  = 0.6f;    // 0..1

private:
    cocos2d::Sprite*      _sprite    = nullptr;
    cocos2d::PhysicsBody* _body      = nullptr;
    cocos2d::Vec2         _moveDir   = cocos2d::Vec2::ZERO;
    int   _footContacts = 0;
    int   _facing       = +1;
    bool  _invincible   = false;

    // Helpers
    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& wantSize);
    void applyPlayerMasks();
    void applyVelocity(float dt);

    // Shim nội bộ để bật/tắt body (dùng bởi enablePhysics(pos,...))
    void enableBody(bool on, const cocos2d::Size& bodySize);
};
