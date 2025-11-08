// Goomba.cpp
#include "game/enemies/Goomba.h"
#include "game/Player.h"

USING_NS_CC;
Goomba* Goomba::create(){ auto p=new(std::nothrow) Goomba(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Goomba::init(){
    if(!Enemy::init()) return false;
    
    // TẠO SPRITE CHO RIÊNG GOOMBA
    _sprite = Sprite::create("sprites/skeleton1/idle/idle_1.png");
    _sprite->setScale(1.5f); // tùy chỉnh kích cỡ hiển thị
    _sprite->setPositionY(16.f); // tùy chỉnh vị trí hiển thị nếu cần
    addChild(_sprite, 1);
    return true;
}

void Goomba::updateEnemy(float dt) {
    if (!_body) return;

    Vec2 v = _body->getVelocity();
    bool chasing = false;

    // --- Logic đuổi (Chasing) ---
    // Đuổi khi bật aggro + trong tầm (giới hạn lệch Y để không “hút xuyên tầng”)
    if (_aggroEnabled && _target) {
        float dx = _target->getPositionX() - getPositionX();
        float dy = _target->getPositionY() - getPositionY();
        if (std::abs(dx) <= _aggroRange && std::abs(dy) <= 160.f) {
            _dir = (dx >= 0 ? +1 : -1);
            v.x = _dir * _chaseSpeed;
            chasing = true;
        }
    }

    // --- Logic tuần tra (Patrolling) ---
    // Không đuổi -> tuần tra giữa 2 mốc
    if (!chasing && _pA != _pB) {
        float x = getPositionX();
        if (_dir > 0 && x >= _pB.x) _dir = -1;
        else if (_dir < 0 && x <= _pA.x) _dir = +1;
        v.x = _dir * _moveSpeed;
    }

    // Giới hạn tốc độ rơi
    v.y = std::max(v.y, -900.0f);
    _body->setVelocity(v);

    // --- Logic Animation & Sprite Flip ---
    // Kiểm tra xem enemy có đang di chuyển (v.x != 0) không
    bool isMoving = (v.x != 0); // Enemy di chuyển khi chasing HOẶC patrolling

    if (isMoving) {
        // play walk animation khi đang di chuyển (chasing hoặc patrolling)
        playAnim("walk", 0.12f, 9, "skeleton1");
    } else {
        // đứng yên (chasing false VÀ _pA == _pB) → idle
        playAnim("idle", 0.18f, 9, "skeleton1");
    }

    // Flip sprite theo hướng di chuyển (trừ khi _dir chưa được set, nhưng trong code này _dir luôn là +1/-1 khi di chuyển)
    if (_sprite) _sprite->setFlippedX(_dir > 0);
}
