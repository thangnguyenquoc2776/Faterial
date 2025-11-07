#include "scenes/EndScene.h"
#include "scenes/MenuScene.h"

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

EndScene* EndScene::createWithText(const std::string& msg){
    auto p = new (std::nothrow) EndScene();
    if (p && p->initWithText(msg)) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}

bool EndScene::initWithText(const std::string& msg){
    if(!Scene::init()) return false;

    auto bg = LayerGradient::create(Color4B(6,10,14,255), Color4B(14,22,30,255));
    addChild(bg, -1);

    _buildUI(msg);

    // ESC/ENTER -> về menu
    auto l = EventListenerKeyboard::create();
    l->onKeyReleased = [](EventKeyboard::KeyCode k, Event*){
        if (k==EventKeyboard::KeyCode::KEY_ESCAPE ||
            k==EventKeyboard::KeyCode::KEY_ENTER ||
            k==EventKeyboard::KeyCode::KEY_KP_ENTER) {
            Director::getInstance()->replaceScene(
                TransitionFade::create(0.2f, MenuScene::createScene()));
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    return true;
}

void EndScene::_buildUI(const std::string& msg){
    auto D   = Director::getInstance();
    auto vs  = D->getVisibleSize();
    auto org = D->getVisibleOrigin();

    auto t = Label::createWithSystemFont(msg.empty() ? "THE END" : msg, "Arial", 48);
    t->setAlignment(TextHAlignment::CENTER);
    t->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.55f));
    addChild(t);

    auto hint = Label::createWithSystemFont("[Enter] Back to Menu", "Arial", 20);
    hint->setOpacity(170);
    hint->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.18f));
    addChild(hint);

    auto back = Button::create();
    back->setTitleFontName("Arial"); back->setTitleFontSize(24);
    back->setTitleText("BACK TO MENU");
    back->setScale9Enabled(true); back->ignoreContentAdaptWithSize(false);
    back->setContentSize({300,56});
    back->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.32f));
    auto box = LayerColor::create(Color4B(232,232,236,255));
    box->setContentSize(back->getContentSize());
    box->setIgnoreAnchorPointForPosition(false);
    box->setAnchorPoint({0.5f,0.5f});
    box->setPosition(back->getContentSize()*0.5f);
    back->addChild(box, -1);
    back->addClickEventListener([](Ref*){
        Director::getInstance()->replaceScene(
            TransitionFade::create(0.2f, MenuScene::createScene()));
    });
    addChild(back);
}
