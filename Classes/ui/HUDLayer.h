#pragma once
#include <vector>
#include <string>
#include "cocos2d.h"

class HUDLayer : public cocos2d::Layer {
public:
    CREATE_FUNC(HUDLayer);
    bool init() override;
    void onEnter() override;
    void update(float dt) override; // tự bám camera mặc định

    // ==== API (GameScene/Player gọi vào) ====
    void setLives(int v);
    void setScore(int v);
    void setStars(int have, int need);
    void setZone(int cur, int total);
    void setHP(int cur, int max);

    // Buffs
    int  addBuff(const std::string& name, float durationSec);
    void removeBuff(int id);
    void tick(float dt); // giảm thời gian buff + redraw bar

private:
    // Text
    cocos2d::Label* _lLives = nullptr; // góc trái trên
    cocos2d::Label* _lScore = nullptr; // góc phải trên
    cocos2d::Label* _lStars = nullptr; // góc phải trên (dưới Score)
    cocos2d::Label* _lZone  = nullptr; // góc phải trên (dưới Stars)

    // HP
    cocos2d::DrawNode* _hpBarBG = nullptr;
    cocos2d::DrawNode* _hpBarFG = nullptr;
    cocos2d::Label*    _lHPText = nullptr;
    int _hpCur = 100, _hpMax = 100;

    // Buff strip
    struct BuffUI {
        int id = 0;
        cocos2d::Node*     root = nullptr;
        cocos2d::Label*    name = nullptr;
        cocos2d::DrawNode* bar  = nullptr;
        float dur = 0.f, remain = 0.f;
    };
    std::vector<BuffUI> _buffs;
    int _nextBuffId = 1;

    // State mirror (không bắt buộc nhưng tiện)
    int _lives = 3;
    int _score = 0;
    int _starsHave = 0, _starsNeed = 0;
    int _zoneCur = 1, _zoneTotal = 1;

    // Layout helpers
    void _layout();         // đặt vị trí các label/thanh
    void _layoutBuffs();    // buff strip
    void _redrawHP();       // vẽ lại thanh HP
    void _anchorToCamera(); // đặt origin HUD trùng đáy-trái màn hình hiện tại
};
