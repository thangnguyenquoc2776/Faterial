// Upgrade.cpp
#include "game/objects/Upgrade.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Upgrade* Upgrade::create(){ auto p=new(std::nothrow) Upgrade(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Upgrade::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::UPGRADE);
    auto dn = DrawNode::create();
    dn->drawSolidRect({-10,-10},{10,10}, Color4F(0.2f,1,1,1));
    addChild(dn);
    auto body = PhysicsBody::createBox(Size(20,20));
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask(phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}
