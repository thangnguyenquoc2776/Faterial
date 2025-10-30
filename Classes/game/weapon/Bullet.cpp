#include "game/weapon/Bullet.h"
USING_NS_CC;

Bullet* Bullet::create(const Vec2& dir, float speed, float life){
    auto p=new(std::nothrow) Bullet();
    if(p && p->initWith(dir,speed,life)){ p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}
bool Bullet::initWith(const Vec2& dir, float speed, float life){
    if(!Node::init()) return false;
    _dir = dir.getNormalized();
    _speed = speed; _life = life;

    auto dn = DrawNode::create();
    dn->drawSolidCircle(Vec2::ZERO, 6.f, 0, 24, Color4F(1.f,0.95f,0.1f,1));
    addChild(dn);

    _body = PhysicsBody::createCircle(6.f, PhysicsMaterial(0,0,0));
    _body->setDynamic(true);
    _body->setGravityEnable(false);
    _body->setRotationEnable(false);
    _body->setCategoryBitmask(phys::CAT_BULLET);
    _body->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_ENEMY);
    _body->setContactTestBitmask(phys::all());
    setPhysicsBody(_body);

    scheduleUpdate();
    return true;
}
void Bullet::update(float dt){
    if(_body) _body->setVelocity(_dir * _speed);
    _life -= dt; if(_life<=0.f) removeFromParent();
}
