#include "game/upgrades/UpgradeSystem.h"
using namespace cocos2d;
using namespace upgrades;

static float frand(float a, float b){
    return RandomHelper::random_real(std::min(a,b), std::max(a,b));
}
static int irand(int a, int b){
    return RandomHelper::random_int(std::min(a,b), std::max(a,b));
}

UpgradeSystem::Effect UpgradeSystem::roll(int segIndex, const RollSpec& spec) {
    // Chọn type
    Type T;
    switch (irand(0,4)) {
        case 0: T = Type::MOVE_SPEED;  break;
        case 1: T = Type::JUMP_POWER;  break;
        case 2: T = Type::DAMAGE_MULT; break;
        case 3: T = Type::BULLET_COUNT;break;
        default:T = Type::SLASH_RANGE; break;
    }

    // Biên độ “mềm tay”
    float mag = 0.f;
    switch (T) {
        case Type::MOVE_SPEED:   mag = frand(18.f, 40.f);   break;
        case Type::JUMP_POWER:   mag = frand(24.f, 60.f);   break;
        case Type::DAMAGE_MULT:  mag = frand(1.15f, 1.45f); break;
        case Type::BULLET_COUNT: mag = (float)irand(1,2);   break;
        case Type::SLASH_RANGE:  mag = frand(1.15f, 1.40f); break;
    }

    // 50% TIMED, còn lại theo defaultScope
    Scope sc  = spec.defaultScope;
    float dur = 0.f;
    if (spec.allowTimed && RandomHelper::random_int(0,1)==1) {
        sc  = Scope::TIMED;
        dur = spec.timedDuration + segIndex * 3.f; // mini sau lâu hơn chút
    }

    return Effect{ T, mag, sc, dur };
}
