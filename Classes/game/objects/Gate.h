// Gate.h
#pragma once
#include "game/Entity.h"
class Gate : public Entity {
public:
    static Gate* create();
    bool init() override;
    void open(bool on);
    bool isOpen() const { return _open; }
private:
    bool _open=false;
    cocos2d::DrawNode* _dn=nullptr;
};
