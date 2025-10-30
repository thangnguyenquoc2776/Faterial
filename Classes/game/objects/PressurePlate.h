// PressurePlate.h
#pragma once
#include "game/Entity.h"
class PressurePlate : public Entity {
public:
    static PressurePlate* create();
    bool init() override;
    void setCallback(const std::function<void(bool)>& cb){ _cb = cb; }
private:
    std::function<void(bool)> _cb;
    int _pressCount = 0;
};
