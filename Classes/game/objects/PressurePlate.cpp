#include "game/objects/PressurePlate.h"
#include "physics/PhysicsDefs.h"
#include "cocos2d.h"

USING_NS_CC;

bool PressurePlate::init(){
    if(!Node::init()) return false;

    const float ww = 48.f, hh = 10.f;
    auto body = PhysicsBody::createBox(Size(ww, hh));
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_SENSOR);
    body->setCollisionBitmask(phys::CAT_PLAYER | phys::CAT_ENEMY);
    body->setContactTestBitmask(phys::CAT_PLAYER | phys::CAT_ENEMY);
    setPhysicsBody(body);

    setTagEx(phys::Tag::PLATE);

    auto dn = DrawNode::create();
    dn->drawSolidRect(Vec2(-ww/2,-hh/2), Vec2(ww/2,hh/2), Color4F(0.9f,0.2f,0.2f,0.9f));
    addChild(dn);

    return true;
}
