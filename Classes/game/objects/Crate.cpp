// Crate.cpp
#include "game/objects/Crate.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Crate* Crate::create(){ auto p=new(std::nothrow) Crate(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Crate::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::CRATE);
    auto dn = DrawNode::create();
    dn->drawSolidRect({-16,-16},{16,16}, Color4F(0.6f,0.4f,0.2f,1));
    addChild(dn);
    auto b = PhysicsBody::createBox(Size(32,32), PhysicsMaterial(0.3f,0,0.9f));
    b->setDynamic(true);
    b->setCategoryBitmask(phys::CAT_CRATE);
    b->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_PLAYER|phys::CAT_ENEMY|phys::CAT_CRATE);
    b->setContactTestBitmask(phys::all());
    setPhysicsBody(b);
    return true;
}
