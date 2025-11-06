// Coin.h
#pragma once
#include "game/Entity.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"
class Coin : public Entity {
public:
    static Coin* create();
    bool init() override;
    cocos2d::Sprite* _sprite = nullptr;
};
