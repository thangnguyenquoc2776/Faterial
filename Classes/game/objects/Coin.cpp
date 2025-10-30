// Coin.cpp
#include "game/objects/Coin.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Coin* Coin::create(){ auto p=new(std::nothrow) Coin(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Coin::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::COIN);
    auto dn = DrawNode::create();
    dn->drawSolidCircle(Vec2::ZERO, 10, 0, 24, Color4F(1,0.85f,0,1));
    addChild(dn);
    auto body = PhysicsBody::createCircle(10);
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_ITEM);
    body->setCollisionBitmask(0);               // sensor-only
    body->setContactTestBitmask(phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}
