// Crate.h
#pragma once
#include "game/Entity.h"
class Crate : public Entity {
public:
    static Crate* create();
    bool init() override;
};
