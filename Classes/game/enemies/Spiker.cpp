// Spiker.cpp
#include "game/enemies/Spiker.h"
USING_NS_CC;
Spiker* Spiker::create(){ auto p=new(std::nothrow) Spiker(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Spiker::init(){
    if(!Enemy::init()) return false;
    setColor(Color3B(180,200,255));
    return true;
}
