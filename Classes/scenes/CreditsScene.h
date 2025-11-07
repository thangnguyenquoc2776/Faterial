#pragma once
#include "cocos2d.h"

class CreditsScene : public cocos2d::Scene {
public:
    CREATE_FUNC(CreditsScene);
    static cocos2d::Scene* createScene();

    bool init() override;
    void onExit() override;

private:
    void _buildUI();
    void _bindInput();

    cocos2d::EventListenerKeyboard* _kb = nullptr;
};
