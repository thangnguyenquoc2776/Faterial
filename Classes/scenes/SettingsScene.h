#pragma once
#include "cocos2d.h"

class SettingsScene : public cocos2d::Scene {
public:
    CREATE_FUNC(SettingsScene);
    static cocos2d::Scene* createScene();

    bool init() override;
    void onExit() override;

private:
    void _buildUI();
    void _bindInput();
    void _loadValues();
    void _saveValues();

    void _applyMusic(float v); // chỉ lưu vào UserDefault (không gọi AudioEngine)
    void _applySfx(float v);

    float _music = 0.8f;
    float _sfx   = 0.8f;

    cocos2d::EventListenerKeyboard* _kb = nullptr;
};
