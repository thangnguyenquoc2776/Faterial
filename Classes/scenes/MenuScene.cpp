#include "scenes/MenuScene.h"
#include "scenes/GameScene.h"
USING_NS_CC;

Scene* MenuScene::createScene(){ return MenuScene::create(); }

bool MenuScene::init(){
    if(!Scene::init()) return false;

    auto vs = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto title = Label::createWithSystemFont("MyGame", "Arial", 48);
    title->setPosition(origin + Vec2(vs.width/2, vs.height*0.65f));
    auto hint  = Label::createWithSystemFont("[ENTER] New Game   |   [ESC] Quit","Arial",24);
    hint->setPosition(origin + Vec2(vs.width/2, vs.height*0.45f));
    addChild(title); addChild(hint);

    // Dự phòng: nếu có Menu* close từ template bị add nhầm
    for (auto ch : getChildren())
        if (auto m = dynamic_cast<Menu*>(ch)) m->setEnabled(false);

    _bindInput();
    return true;
}

void MenuScene::_bindInput(){
    auto l = EventListenerKeyboard::create();
    l->onKeyPressed = [this](EventKeyboard::KeyCode c, Event*){
        if (c==EventKeyboard::KeyCode::KEY_ENTER || c==EventKeyboard::KeyCode::KEY_KP_ENTER) _goGame();
        else if (c==EventKeyboard::KeyCode::KEY_ESCAPE) Director::getInstance()->end();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;  // lưu lại để gỡ
}

void MenuScene::onExit(){
    if (_kb) {
        _eventDispatcher->removeEventListener(_kb);
        _kb = nullptr;
    }
    Scene::onExit();
}

void MenuScene::_goGame(){
    Director::getInstance()->replaceScene(TransitionFade::create(0.25f, GameScene::createScene()));
}
