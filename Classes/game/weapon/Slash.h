#pragma once
#include "cocos2d.h"
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"

#pragma once
#include "game/Entity.h"

class Slash : public Entity {
public:
    static Slash* create(const cocos2d::Vec2& origin, float angleRad,
                         float sizeOrRange, float durationSec = 0.15f);
    bool init() override { return Entity::init(); }
};
