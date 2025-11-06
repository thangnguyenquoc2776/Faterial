#include "ui/HUDLayer.h"
using namespace cocos2d;

bool HUDLayer::init(){
    if(!Layer::init()) return false;

    // text góc
    _lLives = Label::createWithSystemFont("HP: ❤❤❤", "Arial", 22);
    _lScore = Label::createWithSystemFont("Score: 0", "Arial", 22);
    _lStars = Label::createWithSystemFont("★ 0/5", "Arial", 22);
    _lZone  = Label::createWithSystemFont("Zone 1/5", "Arial", 22);
    for(auto* L : {_lLives,_lScore,_lStars,_lZone}){
        L->setColor(Color3B::WHITE);
        L->enableShadow(Color4B(0,0,0,128), Size(1,-1), 1);
        addChild(L);
    }

    // HP bar (trên cùng, trái)
    _hpBarBG = DrawNode::create(); addChild(_hpBarBG);
    _hpBarFG = DrawNode::create(); addChild(_hpBarFG);
    _lHPText = Label::createWithSystemFont("100/100", "Arial", 18);
    _lHPText->enableShadow();
    addChild(_lHPText);

    schedule([this](float dt){ tick(dt); }, "hud.tick");
    _layout();
    _redrawHP();
    return true;
}

void HUDLayer::onEnter(){
    Layer::onEnter();
    _layout();
}

void HUDLayer::_layout(){
    const auto vs=Director::getInstance()->getVisibleSize();
    const auto org=Director::getInstance()->getVisibleOrigin();

    // text
    _lLives->setAnchorPoint({0,1});  _lLives->setPosition(org + Vec2(16, vs.height-12));
    _lScore->setAnchorPoint({0,1});  _lScore->setPosition(org + Vec2(16, vs.height-40));

    _lStars->setAnchorPoint({1,1});  _lStars->setPosition(org + Vec2(vs.width-16, vs.height-12));
    _lZone ->setAnchorPoint({1,1});  _lZone ->setPosition(org + Vec2(vs.width-16, vs.height-40));

    // HP bar
    const float x = org.x + 16;
    const float y = org.y + vs.height - 72;
    const Size  sz(220, 14);

    _hpBarBG->clear();
    _hpBarBG->drawSolidRect({x-2,y-2}, {x+sz.width+2, y+sz.height+2}, Color4F(0,0,0,0.6f));
    _hpBarFG->setPosition(Vec2::ZERO);
    _lHPText->setAnchorPoint({0,0.5f});
    _lHPText->setPosition({x, y-18});

    _redrawHP();

    // Buff strip dưới HP
    _layoutBuffs();
}

void HUDLayer::_redrawHP(){
    const auto vs=Director::getInstance()->getVisibleSize();
    const auto org=Director::getInstance()->getVisibleOrigin();
    const float x = org.x + 16;
    const float y = org.y + vs.height - 72;
    const Size  sz(220, 14);

    float t = (_hpMax>0) ? std::max(0.f, std::min(1.f, _hpCur/(float)_hpMax)) : 0.f;
    _hpBarFG->clear();
    _hpBarFG->drawSolidRect({x,y}, {x+sz.width*t, y+sz.height}, Color4F(0.9f,0.2f,0.2f,1.f));
    _lHPText->setString(StringUtils::format("%d/%d", _hpCur,_hpMax));
}

void HUDLayer::setLives(int v){
    _lives = std::max(0,v);
    std::string hearts;
    for(int i=0;i<_lives;i++) hearts += u8"❤";
    if(hearts.empty()) hearts="0";
    _lLives->setString("HP: " + hearts);
}
void HUDLayer::setScore(int v){ _lScore->setString("Score: " + std::to_string(v)); }
void HUDLayer::setStars(int have, int need){ _lStars->setString("★ " + std::to_string(have)+"/"+std::to_string(need)); }
void HUDLayer::setZone(int cur, int total){ _lZone->setString("Zone " + std::to_string(cur)+"/"+std::to_string(total)); }

void HUDLayer::setHP(int cur, int max){
    _hpCur=std::max(0,cur); _hpMax=std::max(1,max); _redrawHP();
}

// ===== Buff strip =====
int HUDLayer::addBuff(const std::string& name, float dur){
    BuffUI ui; ui.id=_nextBuffId++; ui.dur=dur; ui.remain=dur;
    ui.root = Node::create(); addChild(ui.root);
    ui.name = Label::createWithSystemFont(name, "Arial", 18);
    ui.name->enableShadow();
    ui.bar  = DrawNode::create();

    ui.root->addChild(ui.name);
    ui.root->addChild(ui.bar);
    _buffs.push_back(ui);
    _layoutBuffs();
    return ui.id;
}
void HUDLayer::removeBuff(int id){
    for(size_t i=0;i<_buffs.size();++i){
        if(_buffs[i].id==id){
            _buffs[i].root->removeFromParent();
            _buffs.erase(_buffs.begin()+i);
            break;
        }
    }
    _layoutBuffs();
}
void HUDLayer::_layoutBuffs(){
    const auto vs=Director::getInstance()->getVisibleSize();
    const auto org=Director::getInstance()->getVisibleOrigin();
    const float startY = org.y + vs.height - 110;
    const float centerX= org.x + vs.width * 0.5f;

    const float itemW=220, itemH=18, gap=6;
    float totalW = (float)_buffs.size()*itemW + std::max(0,(int)_buffs.size()-1)*gap;

    float x0 = centerX - totalW*0.5f;
    for(size_t i=0;i<_buffs.size();++i){
        auto& b = _buffs[i];
        b.root->setPosition({x0 + i*(itemW+gap), startY});
        b.name->setAnchorPoint({0,0.5f});
        b.name->setPosition({0, itemH+6});
        // redraw bar
        b.bar->clear();
        b.bar->drawSolidRect({0,0}, {itemW, itemH}, Color4F(0,0,0,0.55f));
        float t = (b.dur>0)? (b.remain / b.dur) : 0.f;
        t = std::max(0.f,std::min(1.f,t));
        b.bar->drawSolidRect({0,0}, {itemW*t, itemH}, Color4F(0.2f,0.8f,1.f,0.9f));
    }
}
void HUDLayer::tick(float dt){
    bool changed=false;
    for(auto& b : _buffs){
        if(b.dur>0){
            b.remain -= dt;
            if(b.remain < 0) b.remain = 0;
        }
    }
    // remove hết buff hết giờ
    for(int i=(int)_buffs.size()-1;i>=0;--i){
        if(_buffs[i].dur>0 && _buffs[i].remain<=0.f){
            _buffs[i].root->removeFromParent();
            _buffs.erase(_buffs.begin()+i);
            changed=true;
        }
    }
    if(changed) _layoutBuffs();
    else _layoutBuffs(); // vẫn redraw chiều dài bar
}
