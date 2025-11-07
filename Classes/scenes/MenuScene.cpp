#include "scenes/MenuScene.h"
#include "scenes/GameScene.h"

// Chỉ 2 include này là đủ cho mọi thứ trong file
#include "cocos2d.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

// --- Bật/tắt Settings & Credits theo macro (tránh __has_include) ---
#ifndef USE_SETTINGS_SCENE
#define USE_SETTINGS_SCENE 0   // đổi thành 1 nếu bạn đã có scenes/SettingsScene.h
#endif
#ifndef USE_CREDITS_SCENE
#define USE_CREDITS_SCENE 0    // đổi thành 1 nếu bạn đã có scenes/CreditsScene.h
#endif

#if USE_SETTINGS_SCENE
  #include "scenes/SettingsScene.h"
#endif
#if USE_CREDITS_SCENE
  #include "scenes/CreditsScene.h"
#endif
// -------------------------------------------------------------------

Scene* MenuScene::createScene(){ return MenuScene::create(); }

bool MenuScene::init(){
    if(!Scene::init()) return false;

    _buildUI();
    _disableLegacyMenus();  // nếu template cũ có Menu* thì vô hiệu
    _bindInput();
    return true;
}

void MenuScene::_buildUI(){
    auto* D  = Director::getInstance();
    auto  vs = D->getVisibleSize();
    auto  org= D->getVisibleOrigin();

    // Nền gradient nhẹ (dùng class từ cocos2d.h)
    auto bg = LayerGradient::create(Color4B(8,12,20,255), Color4B(16,20,28,255));
    addChild(bg, -10);

    auto title = Label::createWithSystemFont("FATERIAL", "Arial", 52);
    title->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.74f));
    title->setOpacity(0);
    title->runAction(FadeIn::create(0.25f));
    addChild(title);

    auto tagline = Label::createWithSystemFont(
        "Mini-zones • Stars • Boss • Bullet vs Projectile",
        "Arial", 18);
    tagline->setOpacity(180);
    tagline->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.68f));
    addChild(tagline);

    // helper tạo nút phẳng (không cần sprite ngoài)
// helper tạo nút phẳng (không cần sprite ngoài)
    auto makeBtn = [&](const std::string& text, float y, std::function<void()> cb){
        auto btn = ui::Button::create();
        btn->setTitleFontName("Arial");
        btn->setTitleFontSize(26);
        btn->setTitleText(text);

        // QUAN TRỌNG: để size có hiệu lực với UI::Widget
        btn->setScale9Enabled(true);
        btn->ignoreContentAdaptWithSize(false);
        btn->setContentSize(Size(300, 56));

        // anchor ở giữa và đặt đúng tâm
        btn->setAnchorPoint(Vec2(0.5f, 0.5f));
        auto* D = cocos2d::Director::getInstance();
        auto  vs = D->getVisibleSize();
        auto  org= D->getVisibleOrigin();
        btn->setPosition(org + Vec2(vs.width * 0.5f, y));
        btn->setZoomScale(0.06f);

        // NỀN hộp sáng — căn GIỮA nút (không để (0,0) nữa)
        auto box = LayerColor::create(Color4B(232,232,236,255));
        box->setContentSize(btn->getContentSize());
        box->setIgnoreAnchorPointForPosition(false);
        box->setAnchorPoint(Vec2(0.5f, 0.5f));
        box->setPosition(btn->getContentSize() * 0.5f);
        btn->addChild(box, -1);

        btn->addClickEventListener([cb](Ref*){ cb(); });
        addChild(btn);
        return btn;
    };


    makeBtn("PLAY",     vs.height*0.50f, [this]{ _goGame();     });
    makeBtn("SETTINGS", vs.height*0.40f, [this]{ _goSettings(); });
    makeBtn("CREDITS",  vs.height*0.30f, [this]{ _goCredits();  });
    makeBtn("QUIT",     vs.height*0.20f, [this]{ _quit();       });

    auto hint = Label::createWithSystemFont(
        "ENTER=Play • S=Settings • C=Credits • ESC=Quit",
        "Arial", 18);
    hint->setOpacity(170);
    hint->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.09f));
    addChild(hint);
}

void MenuScene::_disableLegacyMenus(){
    for (auto ch : getChildren())
        if (auto m = dynamic_cast<Menu*>(ch)) m->setEnabled(false);
}

void MenuScene::_bindInput(){
    auto l = EventListenerKeyboard::create();
    l->onKeyReleased = [this](EventKeyboard::KeyCode k, Event*){
        switch(k){
            case EventKeyboard::KeyCode::KEY_ENTER:
            case EventKeyboard::KeyCode::KEY_KP_ENTER: _goGame(); break;
            case EventKeyboard::KeyCode::KEY_S:         _goSettings(); break;
            case EventKeyboard::KeyCode::KEY_C:         _goCredits();  break;
            case EventKeyboard::KeyCode::KEY_ESCAPE:    _quit();       break;
            default: break;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;
}

void MenuScene::onExit(){
    if (_kb) { _eventDispatcher->removeEventListener(_kb); _kb = nullptr; }
    Scene::onExit();
}

// ----------------- Actions -----------------
void MenuScene::_goGame(){
    Director::getInstance()->replaceScene(TransitionFade::create(0.25f, GameScene::createScene()));
}
void MenuScene::_goSettings(){
#if USE_SETTINGS_SCENE
    Director::getInstance()->pushScene(TransitionFade::create(0.20f, SettingsScene::create()));
#else
    _toastWIP("Settings scene chưa được thêm (WIP). Đặt USE_SETTINGS_SCENE=1 nếu đã có.");
#endif
}
void MenuScene::_goCredits(){
#if USE_CREDITS_SCENE
    Director::getInstance()->pushScene(TransitionFade::create(0.20f, CreditsScene::create()));
#else
    _toastWIP("Credits scene chưa được thêm (WIP). Đặt USE_CREDITS_SCENE=1 nếu đã có.");
#endif
}
void MenuScene::_quit(){
    Director::getInstance()->end();
}

// ----------------- Helper -----------------
void MenuScene::_toastWIP(const std::string& msg){
    auto* D = Director::getInstance();
    auto vs  = D->getVisibleSize();
    auto org = D->getVisibleOrigin();

    auto toast = Label::createWithSystemFont(msg, "Arial", 20);
    toast->setTextColor(Color4B(255,240,180,255));
    toast->enableOutline(Color4B(0,0,0,160), 2);
    toast->setOpacity(0);
    toast->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.12f));
    addChild(toast, 999);

    toast->runAction(Sequence::create(
        FadeIn::create(0.12f),
        DelayTime::create(1.1f),
        FadeOut::create(0.25f),
        RemoveSelf::create(),
        nullptr
    ));
}
