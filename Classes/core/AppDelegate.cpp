// AppDelegate.cpp
#include "core/AppDelegate.h"
#include "base/CCDirector.h"
#include "platform/CCGLView.h"
#include "scenes/MenuScene.h"

// Âm thanh
#include "audio/include/AudioEngine.h"
#include "audio/Sound.h"

USING_NS_CC;
using AE = cocos2d::AudioEngine;

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {
    // Giải phóng AudioEngine an toàn khi thoát app
    AE::end();
}

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs a{8,8,8,8,24,8};
    GLView::setGLContextAttrs(a);
}

bool AppDelegate::applicationDidFinishLaunching() {
    auto* director = Director::getInstance();
    auto* glview   = director->getOpenGLView();
    if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        glview = GLViewImpl::createWithRect("Faterial", Rect(0,0,1920,1080));
#else
        glview = GLViewImpl::create("Faterial");
#endif
        director->setOpenGLView(glview);
    }

    director->setAnimationInterval(1.0f/60.0f);
#if COCOS2D_DEBUG
    director->setDisplayStats(true);
#endif

    // Thiết lập thiết kế màn hình
    glview->setDesignResolutionSize(1280, 720, ResolutionPolicy::NO_BORDER);

    // Khởi tạo âm thanh (đọc volume đã lưu, preload sfx)
    snd::init();

    // Chạy vào Menu
    director->runWithScene(MenuScene::createScene());
    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    // Tạm dừng render + âm thanh
    Director::getInstance()->stopAnimation();
    AE::pauseAll();
}

void AppDelegate::applicationWillEnterForeground() {
    // Tiếp tục render + âm thanh
    Director::getInstance()->startAnimation();
    AE::resumeAll();
}
