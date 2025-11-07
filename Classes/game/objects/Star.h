// Star.h
#pragma once
#include "game/Entity.h"
class Star : public Entity {
public:
    static Star* create();
    bool init() override;
    cocos2d::Sprite* _sprite = nullptr;
};
