// BossGolem.cpp
#include "game/bosses/BossGolem.h"
USING_NS_CC;
BossGolem* BossGolem::create(){ auto p=new(std::nothrow) BossGolem(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool BossGolem::init(){
    if(!Enemy::init()) return false;
    setTextureRect(Rect(0,0,72,72));
    setColor(Color3B(90,70,50));
    _hp = 20; _speed = 60.f;
    return true;
}
void BossGolem::takeHit(int dmg){
    _hp -= dmg;
    setColor((_hp%2==0)?Color3B(120,90,60):Color3B(90,70,50));
    if(_hp<=0) removeFromParent();
}
