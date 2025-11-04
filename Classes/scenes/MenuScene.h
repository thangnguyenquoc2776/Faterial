#pragma once
#include "cocos2d.h"

class MenuScene : public cocos2d::Scene {
public:
    CREATE_FUNC(MenuScene);
    static cocos2d::Scene* createScene();
    bool init() override;
    void onExit() override;                // <-- thêm

private:
    void _bindInput();
    void _goGame();

    cocos2d::EventListenerKeyboard* _kb = nullptr;  // <-- thêm
};
