#pragma once
#include "cocos2d.h"
#include "game/Enemy.h"

namespace levels {

struct BuildResult {
    float groundTop = 0.f;
    int   segments  = 5;
    float segmentWidth = 0.f;
    cocos2d::Vec2 playerSpawn{0,0};
    cocos2d::Vector<Enemy*> enemies;
};

BuildResult buildLevel1(cocos2d::Node* root,
                        const cocos2d::Size& vs,
                        const cocos2d::Vec2& origin);

} // namespace levels
