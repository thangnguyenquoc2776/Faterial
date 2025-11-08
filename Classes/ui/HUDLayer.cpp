#include "ui/HUDLayer.h"
using namespace cocos2d;

namespace {
    constexpr float MARGIN   = 16.f;
    constexpr float LINE_H   = 26.f;
    constexpr float HP_W     = 240.f;
    constexpr float HP_H     = 16.f;
}

bool HUDLayer::init() {
    if (!Layer::init()) return false;

    // Đảm bảo vẽ sau world
    this->setLocalZOrder(9999);

    // ---- Góc trái: Lives ----
    _lLives = Label::createWithSystemFont("Lives: ❤❤❤", "Arial", 22);
    _lLives->setColor(Color3B::WHITE);
    _lLives->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_lLives);

    // ---- HP bar + số ----
    _hpBarBG = DrawNode::create(); addChild(_hpBarBG);
    _hpBarFG = DrawNode::create(); addChild(_hpBarFG);
    _lHPText = Label::createWithSystemFont("100/100", "Arial", 18);
    _lHPText->setColor(Color3B::WHITE);
    _lHPText->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_lHPText);

    // ---- Góc phải: Score/Stars/Zone ----
    _lScore = Label::createWithSystemFont("Score: 0", "Arial", 22);
    _lScore->setColor(Color3B::WHITE);
    _lScore->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_lScore);

    _lStars = Label::createWithSystemFont(u8"★ 0/0", "Arial", 22);
    _lStars->setColor(Color3B::WHITE);
    _lStars->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_lStars);

    _lZone  = Label::createWithSystemFont("Zone 1/1", "Arial", 22);
    _lZone->setColor(Color3B::WHITE);
    _lZone->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_lZone);

    // Buff timer
    schedule([this](float dt){ tick(dt); }, "hud.tick");

    // HUD tự bám camera mặc định
    scheduleUpdate();

    _layout();
    _redrawHP();
    return true;
}

void HUDLayer::onEnter() {
    Layer::onEnter();
    setLocalZOrder(9999);
    _layout();
}

void HUDLayer::update(float dt) {
    CC_UNUSED_PARAM(dt);
    _anchorToCamera(); // giữ HUD luôn dính màn hình
}

// ======================================================
// Anchor HUD vào màn hình (không cần CameraFlag)
// ======================================================
void HUDLayer::_anchorToCamera() {
    auto* sc  = this->getScene();
    if (!sc) return;
    auto* cam = sc->getDefaultCamera();
    if (!cam) return;

    const auto vs = Director::getInstance()->getVisibleSize();
    // Đặt gốc HUD tại đáy-trái viewport
    this->setPosition(Vec2(cam->getPositionX() - vs.width  * 0.5f,
                           cam->getPositionY() - vs.height * 0.5f));
}

// ======================================================
// Layout
// ======================================================
void HUDLayer::_layout() {
    const auto vs = Director::getInstance()->getVisibleSize();

    // ----- Trái trên -----
    _lLives->setAnchorPoint({0.f, 1.f});
    _lLives->setPosition({MARGIN, vs.height - MARGIN});

    // HP ngay dưới Lives
    const float hpX = MARGIN;
    const float hpY = vs.height - (MARGIN + LINE_H);
    _hpBarBG->clear();
    _hpBarBG->drawSolidRect({hpX - 2, hpY - 2}, {hpX + HP_W + 2, hpY + HP_H + 2}, Color4F(0,0,0,0.6f));
    _hpBarFG->setPosition(Vec2::ZERO);

    _lHPText->setAnchorPoint({0.f, 0.5f});
    _lHPText->setPosition({hpX, hpY - 18.f});

    // ----- Phải trên (xếp dọc) -----
    const float rightX = vs.width - MARGIN;
    float topY = vs.height - MARGIN;

    _lScore->setAnchorPoint({1.f, 1.f});
    _lScore->setPosition({rightX, topY});

    topY -= LINE_H;
    _lStars->setAnchorPoint({1.f, 1.f});
    _lStars->setPosition({rightX, topY});

    topY -= LINE_H;
    _lZone->setAnchorPoint({1.f, 1.f});
    _lZone->setPosition({rightX, topY});

    // ----- Buff strip: top-center (đổi vị trí tại đây nếu muốn) -----
    _layoutBuffs();

    _redrawHP();
}

void HUDLayer::_redrawHP() {
    const auto vs = Director::getInstance()->getVisibleSize();
    const float hpX = MARGIN;
    const float hpY = vs.height - (MARGIN + LINE_H);

    const float t = (_hpMax > 0) ? std::max(0.f, std::min(1.f, _hpCur / (float)_hpMax)) : 0.f;

    _hpBarFG->clear();
    _hpBarFG->drawSolidRect({hpX, hpY}, {hpX + HP_W * t, hpY + HP_H}, Color4F(0.9f,0.2f,0.2f,1.f));
    _lHPText->setString(StringUtils::format("%d/%d", _hpCur, _hpMax));
}

void HUDLayer::_layoutBuffs() {
    const auto vs  = Director::getInstance()->getVisibleSize();

    const float startY  = vs.height - (MARGIN + 64.f);
    const float centerX = vs.width * 0.5f;

    const float itemW = 220.f;
    const float itemH = 18.f;
    const float gap   = 8.f;

    const float totalW = _buffs.empty() ? 0.f : (float)_buffs.size() * itemW + ((int)_buffs.size() - 1) * gap;
    float x0 = centerX - totalW * 0.5f;

    for (size_t i = 0; i < _buffs.size(); ++i) {
        auto& b = _buffs[i];
        if (!b.root) continue;

        b.root->setPosition({x0 + i * (itemW + gap), startY});

        b.name->setAnchorPoint({0.f, 0.5f});
        b.name->setPosition({0.f, itemH + 6.f});

        b.bar->clear();
        b.bar->drawSolidRect({0,0}, {itemW, itemH}, Color4F(0,0,0,0.55f));
        float t = (b.dur > 0.f) ? std::max(0.f, std::min(1.f, b.remain / b.dur)) : 0.f;
        b.bar->drawSolidRect({0,0}, {itemW * t, itemH}, Color4F(0.2f,0.8f,1.f,0.9f));
    }
}

// ======================================================
// Public setters
// ======================================================
void HUDLayer::setLives(int v) {
    _lives = std::max(0, v);
    std::string hearts;
    if (_lives <= 8) for (int i = 0; i < _lives; ++i) hearts += u8"❤";
    else hearts = "x" + std::to_string(_lives);
    _lLives->setString(std::string("Lives: ") + (hearts.empty() ? "0" : hearts));
}

void HUDLayer::setScore(int v) {
    _score = v;
    _lScore->setString("Score: " + std::to_string(v));
}

void HUDLayer::setStars(int have, int need) {
    _starsHave = have; _starsNeed = need;
    _lStars->setString("★ " + std::to_string(have) + "/" + std::to_string(need));
}

void HUDLayer::setZone(int cur, int total) {
    _zoneCur = cur; _zoneTotal = total;
    _lZone->setString("Zone " + std::to_string(cur) + "/" + std::to_string(total));
}

void HUDLayer::setHP(int cur, int max) {
    _hpCur = std::max(0, cur);
    _hpMax = std::max(1, max);
    _redrawHP();
}

// ======================================================
// Buffs
// ======================================================
int HUDLayer::addBuff(const std::string& name, float durationSec) {
    BuffUI ui;
    ui.id = _nextBuffId++;
    ui.dur = durationSec;
    ui.remain = durationSec;

    ui.root = Node::create();
    this->addChild(ui.root);

    ui.name = Label::createWithSystemFont(name, "Arial", 18);
    ui.name->setColor(Color3B::WHITE);
    ui.name->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    ui.root->addChild(ui.name);

    ui.bar = DrawNode::create();
    ui.root->addChild(ui.bar);

    _buffs.push_back(ui);
    _layoutBuffs();
    return ui.id;
}

void HUDLayer::removeBuff(int id) {
    for (size_t i = 0; i < _buffs.size(); ++i) {
        if (_buffs[i].id == id) {
            if (_buffs[i].root) _buffs[i].root->removeFromParent();
            _buffs.erase(_buffs.begin() + i);
            break;
        }
    }
    _layoutBuffs();
}

// ======================================================
// Tick
// ======================================================
void HUDLayer::tick(float dt) {
    // Giảm thời gian
    for (auto& b : _buffs) {
        if (b.dur > 0.f) {
            b.remain -= dt;
            if (b.remain < 0.f) b.remain = 0.f;
        }
    }
    // Xoá hết hạn
    for (int i = (int)_buffs.size()-1; i >= 0; --i) {
        if (_buffs[i].dur > 0.f && _buffs[i].remain <= 0.f) {
            if (_buffs[i].root) _buffs[i].root->removeFromParent();
            _buffs.erase(_buffs.begin()+i);
        }
    }
    // Vẽ tiến độ mỗi frame
    _layoutBuffs();
}
