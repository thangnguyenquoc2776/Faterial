// Star.cpp
#include "game/objects/Star.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Star* Star::create(){ auto p=new(std::nothrow) Star(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Star::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::STAR);
    auto dn = DrawNode::create();
    Vec2 pts[5];
    for(int i=0;i<5;++i){ float a = CC_DEGREES_TO_RADIANS(72*i-90); pts[i]=Vec2(0,14).rotateByAngle(Vec2::ZERO,a); }
    dn->drawPoly(pts,5,true,Color4F(1,1,0.2f,1));
    addChild(dn);
    auto body = PhysicsBody::createCircle(12);
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask(phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}
