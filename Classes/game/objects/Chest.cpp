#include "game/objects/Chest.h"
#include "game/objects/Coin.h"
#include "game/objects/Upgrade.h"
#include "physics/PhysicsDefs.h"
#include "base/ccRandom.h"
USING_NS_CC;

bool Chest::init(){
    if (!Entity::init()) return false;

    // simple box
    auto dn = DrawNode::create();
    dn->drawSolidRect(Vec2(-14,-10), Vec2(14,10), Color4F(0.6f,0.35f,0.1f,1));
    dn->drawRect(Vec2(-14,-10), Vec2(14,10), Color4F::WHITE);
    addChild(dn);

    auto body = PhysicsBody::createBox(Size(28,20));
    body->setDynamic(false);
    body->setCategoryBitmask((int)phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask((int)phys::CAT_PLAYER);
    setPhysicsBody(body);

    return true;
}

void Chest::open(){
    if (_opened) return;
    _opened = true;

    // rớt 3-5 coin
    int nCoin = RandomHelper::random_int(3,5);
    for (int i=0;i<nCoin;++i){
        auto c = Coin::create();
        c->setPosition(getPosition() + Vec2(RandomHelper::random_real(-12.f,12.f), 6.f));
        if (auto p = getParent()) p->addChild(c, 5);
    }

    // 1 upgrade random
    auto u = Upgrade::createRandom();
    if (u) {
        u->setPosition(getPosition() + Vec2(0, 18));
        if (auto p = getParent()) p->addChild(u, 5);
    }

    runAction(Sequence::create(FadeOut::create(0.05f),
                               CallFunc::create([this]{ removeFromParent(); }),
                               nullptr));
}
