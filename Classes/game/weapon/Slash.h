#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

class Slash : public cocos2d::Node {
public:
    static Slash* create(float w, float h, float lifetime=0.12f);
    bool initWith(float w, float h, float lifetime);
private:
    float _life=0.12f;
};
