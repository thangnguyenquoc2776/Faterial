#pragma once
#include "game/Entity.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

class Chest : public Entity {
public:
    CREATE_FUNC(Chest);
    bool init() override;

    // Mở rương: nhả loot xong tự hủy
    void open();
    cocos2d::Sprite* _sprite = nullptr;

private:
    bool _opened = false;
};
