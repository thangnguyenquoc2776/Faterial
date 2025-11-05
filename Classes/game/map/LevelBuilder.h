#pragma once
#include "cocos2d.h"
#include "game/Enemy.h"   // << thêm để dùng Enemy*

namespace levels {

struct BuildResult {
    float groundTop = 0.f;
    int   segments = 0;
    float segmentWidth = 0.f;
    cocos2d::Vec2 playerSpawn;
    cocos2d::Vector<Enemy*> enemies;  // << đổi từ Vector<Node*> -> Vector<Enemy*>
};

// Xây Level 1 (5 đoạn liên tiếp)
BuildResult buildLevel1(cocos2d::Node* root,
                        const cocos2d::Size& vs,
                        const cocos2d::Vec2& origin);

} // namespace levels
