#pragma once
#include "cocos2d.h"
#include "game/Enemy.h"

namespace levels {

struct LevelBuildResult {
    float               groundTop = 0.f;     // y của đỉnh mặt đất (nơi đứng)
    int                 segments   = 1;      // số đoạn liên tiếp
    float               segmentWidth = 0.f;  // bề rộng mỗi đoạn (thường = visibleSize.width)
    cocos2d::Vec2       playerSpawn;         // vị trí spawn player
    cocos2d::Vector<Enemy*> enemies;         // trả về tham chiếu các enemy đã add
};

// Xây map 5 đoạn liên tiếp: ground + các bậc thang leo hợp lý, coin/star/enemy
LevelBuildResult buildLevel1(cocos2d::Node* parent,
                             const cocos2d::Size& vs,
                             const cocos2d::Vec2& origin);

} // namespace levels
