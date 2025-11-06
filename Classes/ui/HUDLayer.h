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
    void setLives(int v);
    void setScore(int v);
    void setStars(int have, int need);
    void setZone(int cur, int total);
    void setHP(int cur, int max);

    // Buff UI: trả về id nếu muốn quản lý thủ công; bình thường cứ để HUD tự đếm
    int  addBuff(const std::string& name, float durationSec);
    void removeBuff(int id);
    void tick(float dt);

private:
    cocos2d::Label* _lLives = nullptr;
    cocos2d::Label* _lScore = nullptr;
    cocos2d::Label* _lStars = nullptr;
    cocos2d::Label* _lZone  = nullptr;

    // HP bar (trên trái)
    cocos2d::DrawNode* _hpBarBG = nullptr;
    cocos2d::DrawNode* _hpBarFG = nullptr;
    cocos2d::Label*    _lHPText = nullptr;
    int _hpCur = 100, _hpMax = 100;

    // Buff strip (trên cùng, giữa)
    struct BuffUI {
        int id;
        cocos2d::Node*     root;
        cocos2d::Label*    name;
        cocos2d::DrawNode* bar;
        float dur, remain;
    };
    std::vector<BuffUI> _buffs;
    int _nextBuffId = 1;

    int _lives = 3;

    void _layout();
    void _layoutBuffs();
    void _redrawHP();
};
