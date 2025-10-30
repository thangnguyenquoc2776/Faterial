// Spiker.h
#pragma once
#include "game/Enemy.h"
class Spiker : public Enemy {
public:
    static Spiker* create();
    bool init() override;
};
