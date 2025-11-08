#pragma once
#include "cocos2d.h"
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

class Bullet : public Entity {
public:
    CREATE_FUNC(Bullet);

    // Tạo bullet tại origin, bay với velocity, tự hủy sau lifeSec
    static Bullet* create(const cocos2d::Vec2& origin,
                          const cocos2d::Vec2& velocity,
                          float lifeSec);

protected:
    bool init() override { return Entity::init(); }

private:
    cocos2d::Sprite* _sprite;
};
