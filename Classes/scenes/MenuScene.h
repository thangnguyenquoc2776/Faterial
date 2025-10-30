// MenuScene.h
#pragma once
#include "cocos2d.h"
class MenuScene : public cocos2d::Scene {
public:
    CREATE_FUNC(MenuScene);
    static cocos2d::Scene* createScene();
    bool init() override;
private:
    void _bindInput();
    void _goGame();
};
