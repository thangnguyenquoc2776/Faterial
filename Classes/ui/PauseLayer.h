#pragma once
#include "cocos2d.h"

class PauseLayer : public cocos2d::LayerColor {
public:
    static PauseLayer* create();
    bool init() override;

    void onEnter() override;  // pause toàn game
    void onExit()  override;  // resume toàn game

private:
    void _buildUI();
};
