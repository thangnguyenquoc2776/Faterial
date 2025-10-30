#include "ui/HUDLayer.h"
USING_NS_CC;

bool HUDLayer::init(){
    if(!Layer::init()) return false;
    auto vs = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _lblLives = Label::createWithSystemFont("Lives: 3","Arial",22);
    _lblScore = Label::createWithSystemFont("Score: 0","Arial",22);
    _lblStars = Label::createWithSystemFont("Stars: 0/5","Arial",22);
    _lblZone  = Label::createWithSystemFont("Zone: 1/5","Arial",22);

    _lblLives->setAnchorPoint({0,1});
    _lblLives->setPosition(origin + Vec2(10, vs.height-10));

    _lblScore->setAnchorPoint({1,1});
    _lblScore->setPosition(origin + Vec2(vs.width-10, vs.height-10));

    _lblStars->setAnchorPoint({1,1});
    _lblStars->setPosition(origin + Vec2(vs.width-10, vs.height-40));

    _lblZone->setAnchorPoint({0,1});
    _lblZone->setPosition(origin + Vec2(10, vs.height-40));

    addChild(_lblLives); addChild(_lblScore); addChild(_lblStars); addChild(_lblZone);
    return true;
}
void HUDLayer::setScore(int v){ _score=v; if(_lblScore) _lblScore->setString(StringUtils::format("Score: %d",v)); }
void HUDLayer::setLives(int v){ _lives=v; if(_lblLives) _lblLives->setString(StringUtils::format("Lives: %d",v)); }
void HUDLayer::setStars(int have, int need){
    _stars=have; _need=need;
    if(_lblStars) _lblStars->setString(StringUtils::format("Stars: %d/%d", _stars, _need));
}
void HUDLayer::setZone(int idx, int total){
    _zoneIdx=idx; _zoneTot=total;
    if(_lblZone) _lblZone->setString(StringUtils::format("Zone: %d/%d", _zoneIdx, _zoneTot));
}
