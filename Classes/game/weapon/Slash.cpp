#include "game/weapon/Slash.h"
USING_NS_CC;

Slash* Slash::create(float w, float h, float lifetime){
    auto p=new(std::nothrow) Slash();
    if(p && p->initWith(w,h,lifetime)){ p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}
bool Slash::initWith(float w, float h, float lifetime){
    if(!Node::init()) return false;
    _life=lifetime;

    auto dn=DrawNode::create();
    dn->drawSolidRect({-w/2.f,-h/2.f},{w/2.f,h/2.f}, Color4F(0.2f,1.f,1.f,0.35f));
    addChild(dn);

    auto b = PhysicsBody::createBox(Size(w,h));
    b->setDynamic(false);
    b->setCategoryBitmask(phys::CAT_HITBOX);
    b->setCollisionBitmask(0);
    b->setContactTestBitmask(phys::CAT_ENEMY);
    b->setEnabled(true);
    setPhysicsBody(b);

    runAction(Sequence::create(DelayTime::create(_life), RemoveSelf::create(), nullptr));
    return true;
}
