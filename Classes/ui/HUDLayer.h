#pragma once
#include "cocos2d.h"

class HUDLayer : public cocos2d::Layer {
public:
    CREATE_FUNC(HUDLayer);
    bool init() override;
    void setScore(int v);
    void setLives(int v);
    void setStars(int have, int need);
    void setZone(int idx, int total);

private:
    cocos2d::Label* _lblScore=nullptr;
    cocos2d::Label* _lblLives=nullptr;
    cocos2d::Label* _lblStars=nullptr;
    cocos2d::Label* _lblZone=nullptr;
    int _score=0, _lives=3, _stars=0, _need=5;
    int _zoneIdx=1, _zoneTot=5;
};
