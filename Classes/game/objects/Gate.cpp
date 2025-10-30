// Gate.cpp
#include "game/objects/Gate.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Gate* Gate::create(){ auto p=new(std::nothrow) Gate(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Gate::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::GATE);
    _dn = DrawNode::create();
    _dn->drawSolidRect({-12,-40},{12,40}, Color4F(0.7f,0.7f,0.9f,1));
    addChild(_dn);
    auto b = PhysicsBody::createBox(Size(24,80));
    b->setDynamic(false);
    b->setCategoryBitmask(phys::CAT_GATE);
    b->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_PLAYER|phys::CAT_ENEMY|phys::CAT_CRATE);
    b->setContactTestBitmask(phys::all());
    setPhysicsBody(b);
    return true;
}
void Gate::open(bool on){
    _open = on;
    if(auto* b=getPhysicsBody()){
        b->setEnabled(!on);
    }
    _dn->clear();
    if(on) _dn->drawRect({-12,-40},{12,40}, Color4F(0.2f,1,0.2f,1));     // khung trống
    else   _dn->drawSolidRect({-12,-40},{12,40}, Color4F(0.7f,0.7f,0.9f,1));
}
