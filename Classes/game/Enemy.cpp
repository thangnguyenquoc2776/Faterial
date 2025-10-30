#include "game/Enemy.h"
USING_NS_CC;

Enemy* Enemy::create(){ auto p=new(std::nothrow) Enemy(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Enemy::init(){
    if(!Sprite::init()) return false;
    setTextureRect(Rect(0.f,0.f,42.f,42.f));
    setColor(Color3B::RED);
    scheduleUpdate(); return true;
}
void Enemy::enablePhysics(const Vec2& pos, const Size& sz){
    if(_body) return;
    auto b = PhysicsBody::createBox(sz, PhysicsMaterial(0.1f,0.f,0.f)); // friction thấp
    b->setDynamic(true); b->setRotationEnable(false);
    b->setCategoryBitmask(phys::CAT_ENEMY);
    b->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_PLAYER|phys::CAT_CRATE);
    b->setContactTestBitmask(phys::all());
    setPhysicsBody(b); setPosition(pos);
}
void Enemy::setPatrol(const Vec2& a, const Vec2& b){ _pA=a; _pB=b; _right = (b.x>=a.x); }
void Enemy::takeHit(int dmg){ _hp -= dmg; if(_hp<=0) removeFromParent(); }

void Enemy::_stepPatrol(float){
    if(!_body) return;
    float dir = _right ? 1.f : -1.f;
    auto v = _body->getVelocity(); v.x = dir * _speed; _body->setVelocity(v);
    float x = getPositionX();
    if(_right && x >= _pB.x) _right=false;
    else if(!_right && x <= _pA.x) _right=true;
}

void Enemy::update(float){
    if(!_body) return;
    if(_target){
        float dx = _target->getPositionX() - getPositionX();
        float dy = std::abs(_target->getPositionY() - getPositionY());
        static bool chasing=false;
        float adx = std::abs(dx);

        if(chasing){
            if(adx > _aggroFar) chasing=false;
        }else{
            if(adx < _aggroNear && dy < 140.f) chasing=true;
        }

        if(chasing){
            _right = dx>0.f;
            auto v = _body->getVelocity();
            v.x = (_right?1.f:-1.f) * (_speed*1.35f);
            _body->setVelocity(v);
            return;
        }
    }
    _stepPatrol(0.f);
}
