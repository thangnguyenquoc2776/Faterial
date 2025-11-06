#pragma once
#include "cocos2d.h"

namespace upgrades {

// Thời hạn hiệu lực
enum class Scope : int { TIMED, SEGMENT, PERSIST };

// Loại nâng cấp
enum class Type  : int {
    MOVE_SPEED,     // +px/s
    JUMP_POWER,     // +impulse
    DAMAGE_MULT,    // × sát thương (đạn + chém)
    BULLET_COUNT,   // +số đạn mỗi phát
    SLASH_RANGE     // × tầm chém
};

// Hiệu ứng cụ thể
struct Effect {
    Type  type;
    float magnitude;    // + hoặc × (tuỳ loại)
    Scope scope;
    float durationSec;  // dùng khi TIMED; SEGMENT/PERSIST = 0
};

// Tham số roll gacha
struct RollSpec {
    Scope defaultScope  = Scope::SEGMENT; // mặc định hết mini thì hết
    float timedDuration = 20.f;           // base thời gian khi TIMED
    bool  allowTimed    = true;           // 50% ra TIMED nếu true
};

class UpgradeSystem {
public:
    // Trả về 1 hiệu ứng ngẫu nhiên theo mini (segIndex) và rule
    static Effect roll(int segIndex, const RollSpec& spec = {});
};

} // namespace upgrades
