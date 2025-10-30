#pragma once
#include "cocos2d.h"
#include <cstdint>

namespace phys {
    using Mask = uint32_t;

    enum : Mask {
        CAT_WORLD   = 0x0001,
        CAT_PLAYER  = 0x0002,
        CAT_ENEMY   = 0x0004,
        CAT_ITEM    = 0x0008,
        CAT_GATE    = 0x0010,
        CAT_CRATE   = 0x0020,
        CAT_SENSOR  = 0x0040,
        CAT_BULLET  = 0x0080,   // NEW
        CAT_HITBOX  = 0x0100    // NEW (slash cận chiến)
    };
    inline Mask all(){ return 0xFFFFFFFFu; }

    enum class Tag : int {
        NONE=0, WORLD, PLAYER, ENEMY, ITEM, GATE, CRATE, PLATE, BOSS, STAR, COIN, UPGRADE,
        BULLET, SLASH                                // NEW
    };
}
