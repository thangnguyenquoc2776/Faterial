#include "ui/HUDLayer.h"
#include "2d/CCDrawNode.h"

using namespace cocos2d;

bool HUDLayer::init() {
    if (!Layer::init()) return false;

    // Label cơ bản
    _lLives = Label::createWithSystemFont("Lives: ❤❤❤", "Arial", 22);
    _lScore = Label::createWithSystemFont("Score: 0", "Arial", 22);
    _lStars = Label::createWithSystemFont("★ 0/5", "Arial", 22);
    _lZone  = Label::createWithSystemFont("Zone 1/5", "Arial", 22);
    for (auto* L : {_lLives, _lScore, _lStars, _lZone}) {
        L->setColor(Color3B::WHITE);
        L->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
        addChild(L);
    }

    // HP bar + text
    _hpBar   = DrawNode::create();
    _lHpText = Label::createWithSystemFont("HP 100/100", "Arial", 20);
    _lHpText->setColor(Color3B::WHITE);
    _lHpText->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
    addChild(_hpBar);
    addChild(_lHpText);

    _layout();
    _drawHpBar(100,100);
    return true;
}

void HUDLayer::onEnter() {
    Layer::onEnter();
    _layout();
}

void HUDLayer::_layout() {
    const auto vs  = Director::getInstance()->getVisibleSize();
    const auto org = Director::getInstance()->getVisibleOrigin();

    // Trái trên: HP bar + Lives + Score
    const Vec2 hpOrigin = org + Vec2(16, vs.height - 16);
    _hpBar->setPosition(hpOrigin);
    _lHpText->setAnchorPoint({0,1});
    _lHpText->setPosition(hpOrigin + Vec2(0, -24));

    _lLives->setAnchorPoint({0,1});
    _lLives->setPosition(org + Vec2(16, vs.height - 64));

    _lScore->setAnchorPoint({0,1});
    _lScore->setPosition(org + Vec2(16, vs.height - 92));

    // Phải trên: Stars, Zone
    _lStars->setAnchorPoint({1,1});
    _lStars->setPosition(org + Vec2(vs.width - 16, vs.height - 12));

    _lZone->setAnchorPoint({1,1});
    _lZone->setPosition(org + Vec2(vs.width - 16, vs.height - 40));
}

// ---------- API ----------
void HUDLayer::setLives(int v) {
    v = std::max(0, v);
    std::string hearts;
    for (int i = 0; i < v; ++i) hearts += u8"❤";
    if (hearts.empty()) hearts = "0";
    _lLives->setString("Lives: " + hearts);
}
void HUDLayer::setScore(int v) { _lScore->setString("Score: " + std::to_string(v)); }
void HUDLayer::setStars(int have, int need) { _lStars->setString("★ " + std::to_string(have) + "/" + std::to_string(need)); }
void HUDLayer::setZone(int cur, int total) { _lZone->setString("Zone " + std::to_string(cur) + "/" + std::to_string(total)); }

void HUDLayer::setHP(int cur, int max) {
    cur = std::max(0, std::min(cur, max));
    _lHpText->setString("HP " + std::to_string(cur) + "/" + std::to_string(max));
    _drawHpBar(cur, max);
}

void HUDLayer::_drawHpBar(int cur, int max) {
    _hpBar->clear();
    const float W = 200.f, H = 18.f, pad = 2.f;
    _hpBar->drawSolidRect(Vec2(0, -H), Vec2(W, 0), Color4F(0,0,0,0.45f)); // khung
    _hpBar->drawSolidRect(Vec2(pad, -H+pad), Vec2(W-pad, -pad), Color4F(0.2f,0.2f,0.25f,0.9f)); // nền
    float ratio = (max>0)? (float)cur/(float)max : 0.f;
    float ww = (W-2*pad) * ratio;
    _hpBar->drawSolidRect(Vec2(pad, -H+pad), Vec2(pad+ww, -pad), Color4F(0.2f,0.9f,0.3f,1.f));  // máu
}
