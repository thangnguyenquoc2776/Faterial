// Coin.h
#pragma once
#include "game/Entity.h"
class Coin : public Entity {
public:
    static Coin* create();
    bool init() override;
};
