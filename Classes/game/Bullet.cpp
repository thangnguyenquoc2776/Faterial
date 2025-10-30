#include "game/Bullet.h"
USING_NS_CC;

Bullet* Bullet::create(const Vec2& dir, float speed, float life){
    auto p = new(std::nothrow) Bullet();
    if(p && p->initWith(dir, speed, life)){ p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}
bool Bullet::initWith(const Vec2& dir, float speed, float life){
    if(!Node::init()) return false;
    _dir = dir.getNormalized(); _speed = speed; _life = life;

    // vẽ viên đạn
    auto dn = DrawNode::create(); dn->drawSolidCircle(Vec2::ZERO, 5, 0, 16, Color4F(1,1,0,1));
    addChild(dn);

    auto mat = PhysicsMaterial(0,0,0);
    _body = PhysicsBody::createCircle(5, mat);
    _body->setDynamic(true);
    _body->setGravityEnable(false);
    _body->setRotationEnable(false);
    _body->setLinearDamping(0.f);

    _body->setCategoryBitmask(phys::CAT_BULLET);
    _body->setCollisionBitmask(phys::CAT_WORLD | phys::CAT_ENEMY);
    _body->setContactTestBitmask(phys::all());

    setPhysicsBody(_body);
    schedule(CC_SCHEDULE_SELECTOR(Bullet::_tick));
    return true;
}
void Bullet::_tick(float dt){
    if(!_body) return;
    _body->setVelocity(_dir * _speed);
    _life -= dt; if(_life <= 0.f) removeFromParent();
}
