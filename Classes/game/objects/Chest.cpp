#include "game/objects/Chest.h"
#include "2d/CCDrawNode.h"
#include "physics/CCPhysicsBody.h"
#include "game/loot/LootTable.h"

USING_NS_CC;

bool Chest::init(){
    if (!Enemy::init()) return false;

    // vẽ placeholder chiếc rương (vẽ như top-anchored)
    auto d = DrawNode::create();
    const Size S(36, 28);
    d->drawSolidRect(Vec2(-S.width*0.5f, -S.height), Vec2(S.width*0.5f, 0),
                     Color4F(0.65f,0.45f,0.20f,1.f));
    addChild(d);
    setContentSize(S);
    return true;
}

void Chest::enablePhysicsTop(const Vec2& topCenter, const Size& sz){
    setPosition(topCenter);  // Node = đường TOP

    auto body = PhysicsBody::createBox(sz, PhysicsMaterial(0.2f, 0.f, 0.6f),
                                       Vec2(0, -sz.height*0.5f)); // offset xuống dưới
    body->setDynamic(false);
    body->setRotationEnable(false);

    // Cho rương vào CAT_ENEMY để tái dùng logic Bullet/Slash ↔ Enemy
    const uint32_t CAT_ENEMY = 1u<<2, ALL=0xFFFFFFFFu;
    body->setCategoryBitmask(CAT_ENEMY);
    body->setCollisionBitmask(ALL);
    body->setContactTestBitmask(ALL);
    setPhysicsBody(body);
}

void Chest::takeHit(int dmg){
    hp -= std::max(1, dmg);
    // nháy nhẹ báo trúng
    runAction(Sequence::create(TintTo::create(0, 255,200,200),
                               TintTo::create(0.08f, 255,255,255),
                               nullptr));
    if (hp <= 0) spawnLootAndVanish();
}

void Chest::spawnLootAndVanish(){
    LootTable::rollAndSpawn(getParent(), getPosition(), LootTable::defaultChestTable());
    removeFromParent();
}
