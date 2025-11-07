#include "scenes/SettingsScene.h"
#include "scenes/MenuScene.h"

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* SettingsScene::createScene(){ return SettingsScene::create(); }

bool SettingsScene::init(){
    if(!Scene::init()) return false;

    // nền
    auto bg = LayerGradient::create(Color4B(10,14,22,255), Color4B(18,24,32,255));
    addChild(bg, -1);

    _loadValues();
    _buildUI();
    _bindInput();
    return true;
}

void SettingsScene::_buildUI(){
    auto D   = Director::getInstance();
    auto vs  = D->getVisibleSize();
    auto org = D->getVisibleOrigin();

    auto title = Label::createWithSystemFont("SETTINGS","Arial",48);
    title->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.78f));
    addChild(title);

    auto hint = Label::createWithSystemFont("[ESC] Back to Menu", "Arial", 18);
    hint->setOpacity(170);
    hint->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.12f));
    addChild(hint);

    // ---- THAY THẾ HẲN HÀM mkRow BẰNG BẢN NÀY ----
    auto mkRow = [&](const std::string& name, float y, float* pv, bool isMusic){
        auto lbl = Label::createWithSystemFont(name, "Arial", 26);
        lbl->setAnchorPoint({1,0.5f});
        lbl->setPosition(org + Vec2(vs.width*0.5f - 120, y));
        addChild(lbl);

        auto valLbl = Label::createWithSystemFont(StringUtils::format("%d%%", (int)roundf((*pv)*100)), "Arial", 24);
        valLbl->setAnchorPoint({0.5f,0.5f});
        valLbl->setPosition(org + Vec2(vs.width*0.5f, y));
        addChild(valLbl);

        auto makeBtn = [&](const char* text, const Vec2& pos, std::function<void()> onClick){
            auto b = ui::Button::create();
            b->setTitleFontName("Arial"); b->setTitleFontSize(28);
            b->setTitleText(text);
            b->setScale9Enabled(true);
            b->ignoreContentAdaptWithSize(false);
            b->setContentSize({52,52});
            b->setPosition(pos);
            auto box = LayerColor::create(Color4B(232,232,236,255));
            box->setContentSize(b->getContentSize());
            box->setIgnoreAnchorPointForPosition(false);
            box->setAnchorPoint({0.5f,0.5f});
            box->setPosition(b->getContentSize()*0.5f);
            b->addChild(box, -1);
            b->addClickEventListener([onClick](Ref*){ onClick(); });
            addChild(b);
            return b;
        };

        // Nút -
        makeBtn("-", org + Vec2(vs.width*0.5f + 120, y), [this, pv, valLbl, isMusic](){
            *pv = clampf((*pv) - 0.05f, 0.f, 1.f);
            valLbl->setString(StringUtils::format("%d%%", (int)roundf((*pv)*100)));
            if (isMusic) _applyMusic(*pv); else _applySfx(*pv);
            _saveValues();
        });

        // Nút +
        makeBtn("+", org + Vec2(vs.width*0.5f + 190, y), [this, pv, valLbl, isMusic](){
            *pv = clampf((*pv) + 0.05f, 0.f, 1.f);
            valLbl->setString(StringUtils::format("%d%%", (int)roundf((*pv)*100)));
            if (isMusic) _applyMusic(*pv); else _applySfx(*pv);
            _saveValues();
        });
    };

    // ---- THAY 2 LỜI GỌI CŨ ----
    // mkRow("Music", vs.height*0.58f, _music);
    // mkRow("SFX",   vs.height*0.46f, _sfx);

    // ---- BẰNG 2 LỜI GỌI MỚI DÙNG CON TRỎ ----
    mkRow("Music", vs.height*0.58f, &_music, true);
    mkRow("SFX",   vs.height*0.46f, &_sfx,   false);


    // Nút Back
    auto back = Button::create();
    back->setTitleFontName("Arial"); back->setTitleFontSize(24);
    back->setTitleText("BACK");
    back->setScale9Enabled(true);
    back->ignoreContentAdaptWithSize(false);
    back->setContentSize({220,54});
    back->setPosition(org + Vec2(vs.width*0.5f, vs.height*0.28f));
    auto box = LayerColor::create(Color4B(232,232,236,255));
    box->setContentSize(back->getContentSize());
    box->setIgnoreAnchorPointForPosition(false);
    box->setAnchorPoint({0.5f,0.5f});
    box->setPosition(back->getContentSize()*0.5f);
    back->addChild(box, -1);
    back->addClickEventListener([](Ref*){
        Director::getInstance()->popScene();
    });
    addChild(back);
}

void SettingsScene::_bindInput(){
    auto l = EventListenerKeyboard::create();
    l->onKeyReleased = [](EventKeyboard::KeyCode k, Event*){
        if (k == EventKeyboard::KeyCode::KEY_ESCAPE) {
            Director::getInstance()->popScene();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;
}

void SettingsScene::onExit(){
    if (_kb) { _eventDispatcher->removeEventListener(_kb); _kb=nullptr; }
    Scene::onExit();
}

void SettingsScene::_loadValues(){
    auto ud = UserDefault::getInstance();
    _music = clampf(ud->getFloatForKey("cfg.music", 0.8f), 0.f, 1.f);
    _sfx   = clampf(ud->getFloatForKey("cfg.sfx",   0.8f), 0.f, 1.f);
}
void SettingsScene::_saveValues(){
    auto ud = UserDefault::getInstance();
    ud->setFloatForKey("cfg.music", _music);
    ud->setFloatForKey("cfg.sfx",   _sfx);
    ud->flush();
}
void SettingsScene::_applyMusic(float v){ CCLOG("Music volume=%.2f (saved only)", v); }
void SettingsScene::_applySfx(float v){   CCLOG("SFX   volume=%.2f (saved only)", v); }
