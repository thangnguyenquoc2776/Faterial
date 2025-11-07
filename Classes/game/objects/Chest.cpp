#include "game/objects/Chest.h"
#include "game/objects/Coin.h"
#include "game/objects/Upgrade.h"

#include "physics/PhysicsDefs.h"
#include "physics/CCPhysicsBody.h"

#include "2d/CCDrawNode.h"
#include "base/ccRandom.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"
USING_NS_CC;

bool Chest::init() {
    if (!Entity::init()) return false;

    _sprite = Sprite::create("sprites/objects/chest/chest_1.png");
    _sprite->setScale(1.3f); // tùy chỉnh kích cỡ hiển thị
    addChild(_sprite, 1);

    // Vật lý: item-sensor, chỉ cần contact với Player
    auto body = PhysicsBody::createBox(Size(28, 20));
    body->setDynamic(false);
    body->setCategoryBitmask((int)phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask((int)phys::CAT_PLAYER);
    setPhysicsBody(body);

    return true;
}

void Chest::open() {
    if (_opened) return;
    _opened = true;

    auto* parent = getParent();
    if (!parent) { removeFromParent(); return; }

    // Rơi 3–5 coin rải xung quanh
    int nCoin = RandomHelper::random_int(3, 5);
    for (int i = 0; i < nCoin; ++i) {
        if (auto* c = Coin::create()) {
            c->setPosition(getPosition() + Vec2(
                RandomHelper::random_real(-12.f, 12.f),
                RandomHelper::random_real(6.f, 16.f)
            ));
            parent->addChild(c, 5);
        }
    }

    // 1 Upgrade (hoặc bạn chỉnh tỉ lệ nếu muốn)
    if (auto* u = Upgrade::createRandom()) {
        u->setPosition(getPosition() + Vec2(0, 18));
        parent->addChild(u, 5);
    }

    // KHÔNG spawn Star ở đây
    runAction(Sequence::create(
        FadeOut::create(0.05f),
        CallFunc::create([this] { removeFromParent(); }),
        nullptr
    ));
}
