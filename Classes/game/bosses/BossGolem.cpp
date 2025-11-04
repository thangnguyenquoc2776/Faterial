// Classes/game/bosses/BossGolem.cpp
#include "game/bosses/BossGolem.h"
#include "2d/CCSprite.h"
#include "2d/CCActionInterval.h"

USING_NS_CC;

BossGolem* BossGolem::create() {
    auto p = new (std::nothrow) BossGolem();
    if (p && p->init()) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool BossGolem::init() {
    if (!Enemy::init()) return false;

    // Đảm bảo có sprite hiển thị từ Enemy (Enemy giữ _sprite)
    if (!_sprite) {
        _sprite = Sprite::create();
        _sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _sprite->setContentSize(Size(72.f, 72.f));
        addChild(_sprite);
    }

    _sprite->setTextureRect(Rect(0, 0, 72, 72));
    _sprite->setColor(Color3B(90, 70, 50));

    _hp    = 20;
    _speed = 60.f; // tuỳ bạn có dùng hay không

    return true;
}

void BossGolem::takeHit(int dmg) {
    _hp -= dmg;

    if (_sprite) {
        _sprite->runAction(Sequence::create(
            TintTo::create(0.05f, 200, 160, 120),
            TintTo::create(0.05f, 255, 255, 255),
            nullptr
        ));
    }

    if (_hp <= 0) removeFromParent();
}
