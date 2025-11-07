#pragma once
#include "cocos2d.h"

class MenuScene : public cocos2d::Scene {
public:
    CREATE_FUNC(MenuScene);
    static cocos2d::Scene* createScene();

    bool init() override;
    void onExit() override;

private:
    // UI + input
    void _buildUI();
    void _bindInput();

    // actions
    void _goGame();
    void _goSettings();
    void _goCredits();
    void _quit();

    // helper
    void _disableLegacyMenus();
    void _toastWIP(const std::string& msg);

    cocos2d::EventListenerKeyboard* _kb = nullptr; // gỡ listener khi rời scene
};
