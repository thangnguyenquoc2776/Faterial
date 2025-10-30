#include "game/Enemy.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;

Enemy* Enemy::create(){ auto p=new(std::nothrow) Enemy(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Enemy::init(){
    if(!Sprite::init()) return false;
    setTextureRect(Rect(0.f,0.f,42.f,42.f));
    setColor(Color3B::RED);
    _gfx = this;
    scheduleUpdate();
    return true;
}
void Enemy::enablePhysics(const Vec2& pos, const Size& sz){
    if(_body) return;
    _body = PhysicsBody::createBox(sz, PhysicsMaterial(0.1f,0.f,0.9f));
    _body->setDynamic(true);
    _body->setRotationEnable(false);
    _body->setCategoryBitmask(phys::CAT_ENEMY);
    _body->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_PLAYER|phys::CAT_CRATE);
    _body->setContactTestBitmask(phys::all());
    setPhysicsBody(_body);
    setPosition(pos);
}
void Enemy::setPatrol(const Vec2& a, const Vec2& b){ _pA=a; _pB=b; _right=(b.x>=a.x); }
void Enemy::takeHit(int dmg){ _hp -= dmg; if(_hp<=0) removeFromParent(); }
void Enemy::_stepPatrol(float){
    if(!_body) return;
    float dir = _right ? 1.f : -1.f;
    auto v = _body->getVelocity(); v.x = dir * _speed; _body->setVelocity(v);
    float x = getPositionX();
    if(_right && x >= _pB.x) _right=false;
    else if(!_right && x <= _pA.x) _right=true;
}
void Enemy::update(float dt){
    if(!_body){ return; }
    // Đuổi khi player trong tầm 220px theo trục X
    if(_target){
        float dx = _target->getPositionX() - getPositionX();
        float dy = std::abs(_target->getPositionY() - getPositionY());
        if(std::abs(dx) < 220.f && dy < 120.f){
            _right = dx > 0.f;
            auto v = _body->getVelocity(); v.x = (_right?1.f:-1.f) * (_speed*1.35f); _body->setVelocity(v);
            return;
        }
    }
    _stepPatrol(dt);
}
