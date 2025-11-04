#pragma once
#include "cocos2d.h"

class HUDLayer : public cocos2d::Layer {
public:
    CREATE_FUNC(HUDLayer);

    bool init() override;
    void onEnter() override;

    // ===== API GameScene gọi =====
    void setLives(int v);                    // v trái tim (❤❤❤)
    void setScore(int v);                    // điểm
    void setStars(int have, int need);       // sao đã nhặt / cần
    void setZone(int cur, int total);        // đoạn hiện tại / tổng

private:
    cocos2d::Label* _lLives = nullptr;
    cocos2d::Label* _lScore = nullptr;
    cocos2d::Label* _lStars = nullptr;
    cocos2d::Label* _lZone  = nullptr;

    void _layout();                          // đặt vị trí label theo màn hình
};
