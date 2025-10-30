// Upgrade.h
#pragma once
#include "game/Entity.h"
class Upgrade : public Entity {
public:
    static Upgrade* create();
    bool init() override;
};
