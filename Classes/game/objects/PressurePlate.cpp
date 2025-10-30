// PressurePlate.cpp
#include "game/objects/PressurePlate.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
PressurePlate* PressurePlate::create(){ auto p=new(std::nothrow) PressurePlate(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool PressurePlate::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::PLATE);
    auto dn = DrawNode::create();
    dn->drawSolidRect({-18,-4},{18,4}, Color4F(0.9f,0.4f,0.4f,1));
    addChild(dn);
    auto b = PhysicsBody::createBox(Size(36,8));
    b->setDynamic(false);
    b->setCategoryBitmask(phys::CAT_SENSOR);
    b->setCollisionBitmask(0);
    b->setContactTestBitmask(phys::CAT_PLAYER|phys::CAT_CRATE);
    setPhysicsBody(b);
    return true;
}
