// Goomba.cpp
#include "game/enemies/Goomba.h"
USING_NS_CC;
Goomba* Goomba::create(){ auto p=new(std::nothrow) Goomba(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Goomba::init(){
    if(!Enemy::init()) return false;
    setColor(Color3B(200,120,60));
    return true;
}
