#include "scenes/CreditsScene.h"
#include "scenes/MenuScene.h"

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* CreditsScene::createScene(){ return CreditsScene::create(); }

bool CreditsScene::init(){
    if(!Scene::init()) return false;

    auto bg = LayerGradient::create(Color4B(8,10,16,255), Color4B(14,18,26,255));
    addChild(bg, -1);

    _buildUI();
    _bindInput();
    return true;
}

void CreditsScene::_buildUI(){
    auto D   = Director::getInstance();
    auto vs  = D->getVisibleSize();
    auto org = D->getVisibleOrigin();

    auto title = Label::createWithSystemFont("CREDITS", "Arial", 48);
    title->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.78f));
    addChild(title);

    std::string lines =
        "Game Design: You & Team\n"
        "Programming: You (& Chat teammate)\n"
        "Art: Placeholder Pack\n"
        "Audio: Placeholder Pack\n"
        "Engine: cocos2d-x 4.x\n\n"
        "Special Thanks: Friends & Community";
    auto body = Label::createWithSystemFont(lines, "Arial", 22);
    body->setAlignment(TextHAlignment::CENTER);
    body->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.52f));
    addChild(body);

    auto hint = Label::createWithSystemFont("[ESC] Back to Menu", "Arial", 18);
    hint->setOpacity(170);
    hint->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.12f));
    addChild(hint);

    auto back = Button::create();
    back->setTitleFontName("Arial"); back->setTitleFontSize(24);
    back->setTitleText("BACK");
    back->setScale9Enabled(true); back->ignoreContentAdaptWithSize(false);
    back->setContentSize({220,54});
    back->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.28f));
    auto box = LayerColor::create(Color4B(232,232,236,255));
    box->setContentSize(back->getContentSize());
    box->setIgnoreAnchorPointForPosition(false);
    box->setAnchorPoint({0.5f,0.5f});
    box->setPosition(back->getContentSize()*0.5f);
    back->addChild(box, -1);
    back->addClickEventListener([](Ref*){ Director::getInstance()->popScene(); });
    addChild(back);
}

void CreditsScene::_bindInput(){
    auto l = EventListenerKeyboard::create();
    l->onKeyReleased = [](EventKeyboard::KeyCode k, Event*){
        if (k == EventKeyboard::KeyCode::KEY_ESCAPE) Director::getInstance()->popScene();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;
}

void CreditsScene::onExit(){
    if (_kb) { _eventDispatcher->removeEventListener(_kb); _kb=nullptr; }
    Scene::onExit();
}
