#include "ui/PauseLayer.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "scenes/MenuScene.h"
#include "scenes/SettingsScene.h" // nếu chưa có, có thể comment 2 dòng liên quan
#include "scenes/GameScene.h"

USING_NS_CC;
using namespace cocos2d::ui;

PauseLayer* PauseLayer::create() {
    auto p = new (std::nothrow) PauseLayer();
    if (p && p->init()) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}

bool PauseLayer::init() {
    if (!LayerColor::initWithColor(Color4B(0,0,0,156))) return false;

    auto vs  = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();
    setContentSize(vs);
    setIgnoreAnchorPointForPosition(false);
    setPosition(org);
    setLocalZOrder(9999);

    _buildUI();

    // ESC = Resume
    auto kb = EventListenerKeyboard::create();
    kb->onKeyReleased = [this](EventKeyboard::KeyCode k, Event*){
        if (k == EventKeyboard::KeyCode::KEY_ESCAPE) this->removeFromParent();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

    // chặn input xuyên xuống gameplay
    auto touch = EventListenerTouchOneByOne::create();
    touch->setSwallowTouches(true);
    touch->onTouchBegan = [](Touch*, Event*){ return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);
    return true;
}

void PauseLayer::_buildUI() {
    auto vs  = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();

    auto title = Label::createWithSystemFont("PAUSED", "Arial", 44);
    title->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.70f));
    addChild(title);

    auto makeBtn = [&](const std::string& text, float y, std::function<void()> cb){
        auto btn = Button::create();
        btn->setTitleFontName("Arial");
        btn->setTitleFontSize(26);
        btn->setTitleText(text);
        btn->setScale9Enabled(true);
        btn->ignoreContentAdaptWithSize(false);
        btn->setContentSize(Size(300,56));
        btn->setAnchorPoint(Vec2(0.5f,0.5f));
        btn->setPosition(org + Vec2(vs.width*0.5f, y));
        btn->setZoomScale(0.06f);

        auto box = LayerColor::create(Color4B(232,232,236,255));
        box->setContentSize(btn->getContentSize());
        box->setIgnoreAnchorPointForPosition(false);
        box->setAnchorPoint(Vec2(0.5f,0.5f));
        box->setPosition(btn->getContentSize()*0.5f);
        btn->addChild(box, -1);

        btn->addClickEventListener([cb](Ref*){ cb(); });
        addChild(btn);
    };

    makeBtn("RESUME  [ESC]", vs.height*0.54f, [this]{ this->removeFromParent(); });
    makeBtn("RESTART",       vs.height*0.44f, []{
        Director::getInstance()->resume();
        Director::getInstance()->replaceScene(TransitionFade::create(0.2f, GameScene::createScene()));
    });
    makeBtn("SETTINGS",      vs.height*0.34f, []{
        // nếu chưa có SettingsScene, comment dòng dưới
        Director::getInstance()->pushScene(TransitionFade::create(0.2f, SettingsScene::create()));
    });
    makeBtn("MAIN MENU",     vs.height*0.24f, []{
        Director::getInstance()->resume();
        Director::getInstance()->replaceScene(TransitionFade::create(0.2f, MenuScene::createScene()));
    });
    makeBtn("QUIT",          vs.height*0.14f, []{
        Director::getInstance()->end();
    });
}

void PauseLayer::onEnter() {
    LayerColor::onEnter();
    // Pause “cứng” – dừng scheduler + actions cho mọi thứ bên dưới
    Director::getInstance()->pause();
}

void PauseLayer::onExit() {
    // Resume khi đóng overlay
    Director::getInstance()->resume();
    LayerColor::onExit();
}
