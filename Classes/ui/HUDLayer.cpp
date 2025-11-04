#include "ui/HUDLayer.h"
using namespace cocos2d;

bool HUDLayer::init() {
    if (!Layer::init()) return false;

    // Tạo label
    _lLives = Label::createWithSystemFont("HP: ❤❤❤", "Arial", 22);
    _lScore = Label::createWithSystemFont("Score: 0", "Arial", 22);
    _lStars = Label::createWithSystemFont("★ 0/5", "Arial", 22);
    _lZone  = Label::createWithSystemFont("Zone 1/5", "Arial", 22);

    for (auto* L : {_lLives, _lScore, _lStars, _lZone}) {
        L->setColor(Color3B::WHITE);
        L->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
        addChild(L);
    }

    _layout();
    return true;
}

void HUDLayer::onEnter() {
    Layer::onEnter();
    _layout(); // đảm bảo đúng vị trí khi scene vừa vào
}

void HUDLayer::_layout() {
    const auto vs  = Director::getInstance()->getVisibleSize();
    const auto org = Director::getInstance()->getVisibleOrigin();

    // Trái trên: HP, Score
    _lLives->setAnchorPoint({0,1});
    _lLives->setPosition(org + Vec2(16, vs.height - 12));

    _lScore->setAnchorPoint({0,1});
    _lScore->setPosition(org + Vec2(16, vs.height - 40));

    // Phải trên: Stars, Zone
    _lStars->setAnchorPoint({1,1});
    _lStars->setPosition(org + Vec2(vs.width - 16, vs.height - 12));

    _lZone->setAnchorPoint({1,1});
    _lZone->setPosition(org + Vec2(vs.width - 16, vs.height - 40));
}

// ---------- API cập nhật ----------
void HUDLayer::setLives(int v) {
    v = std::max(0, v);
    std::string hearts;
    for (int i = 0; i < v; ++i) hearts += u8"❤";
    if (hearts.empty()) hearts = "0";
    _lLives->setString("HP: " + hearts);
}

void HUDLayer::setScore(int v) {
    _lScore->setString("Score: " + std::to_string(v));
}

void HUDLayer::setStars(int have, int need) {
    _lStars->setString("★ " + std::to_string(have) + "/" + std::to_string(need));
}

void HUDLayer::setZone(int cur, int total) {
    _lZone->setString("Zone " + std::to_string(cur) + "/" + std::to_string(total));
}
