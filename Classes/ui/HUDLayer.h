#pragma once
#include <vector>
#include <string>
#include "cocos2d.h"

class HUDLayer : public cocos2d::Layer {
public:
    CREATE_FUNC(HUDLayer);
    bool init() override;
    void onEnter() override;

    // ===== API từ GameScene/Player =====
    void setLives(int v);                   // số mạng (hiển thị ❤)
    void setScore(int v);                   // điểm
    void setStars(int have, int need);      // sao đã nhặt / cần
    void setZone(int cur, int total);       // mini hiện tại / tổng
    void setHP(int cur, int max);           // HP hiện tại / tối đa (vẽ thanh đỏ)

    // Buff UI: trả về id để remove thủ công nếu muốn
    int  addBuff(const std::string& name, float durationSec);
    void removeBuff(int id);

    // Mỗi frame update bar buff
    void tick(float dt);

private:
    // ----- Texts -----
    cocos2d::Label* _lLives = nullptr;
    cocos2d::Label* _lScore = nullptr;
    cocos2d::Label* _lStars = nullptr;
    cocos2d::Label* _lZone  = nullptr;

    // ----- HP bar -----
    cocos2d::DrawNode* _hpBarBG = nullptr;
    cocos2d::DrawNode* _hpBarFG = nullptr;
    cocos2d::Label*    _lHPText = nullptr;
    int _hpCur = 100, _hpMax = 100;

    // ----- Buff strip -----
    struct BuffUI {
        int id = 0;
        cocos2d::Node*     root = nullptr;
        cocos2d::Label*    name = nullptr;
        cocos2d::DrawNode* bar  = nullptr;
        float dur = 0.f, remain = 0.f;
    };
    std::vector<BuffUI> _buffs;
    int _nextBuffId = 1;

    // ----- State -----
    int _lives = 3;
    int _score = 0;
    int _starsHave = 0, _starsNeed = 0;
    int _zoneCur = 1, _zoneTotal = 1;

    // ----- Layout helpers -----
    void _layout();         // đặt vị trí các label/khung
    void _layoutBuffs();    // xếp hàng buff strip
    void _redrawHP();       // vẽ lại thanh HP theo _hpCur/_hpMax
};
