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
    void setHP(int cur, int max);            // <--- thêm: thanh HP + số

private:
    cocos2d::Label*   _lLives = nullptr;
    cocos2d::Label*   _lScore = nullptr;
    cocos2d::Label*   _lStars = nullptr;
    cocos2d::Label*   _lZone  = nullptr;

    // HP bar vẽ bằng DrawNode để sau này dễ thay asset
    cocos2d::DrawNode* _hpBar   = nullptr;
    cocos2d::Label*    _lHpText = nullptr;

    void _layout();                          // đặt vị trí label theo màn hình
    void _drawHpBar(int cur, int max);
};
