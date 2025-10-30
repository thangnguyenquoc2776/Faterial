#include "game/SlashHitbox.h"
USING_NS_CC;

SlashHitbox* SlashHitbox::create(const Size& box, float life){
    auto p = new(std::nothrow) SlashHitbox();
    if(p && p->initWith(box, life)){ p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}
bool SlashHitbox::initWith(const Size& box, float life){
    if(!Node::init()) return false;
    _life = life;

    auto b = PhysicsBody::createBox(box);
    b->setDynamic(false);
    b->setCategoryBitmask(phys::CAT_SENSOR);
    b->setCollisionBitmask(0);
    b->setContactTestBitmask(phys::CAT_ENEMY);
    setPhysicsBody(b);

    // viền hiển thị thoáng qua
    auto dn = DrawNode::create();
    dn->drawRect({-box.width/2,-box.height/2},{box.width/2,box.height/2}, Color4F(1,0.2f,0.2f,0.8f));
    addChild(dn);

    schedule(CC_SCHEDULE_SELECTOR(SlashHitbox::_tick));
    return true;
}
void SlashHitbox::_tick(float dt){
    _life -= dt; if(_life <= 0.f) removeFromParent();
}
