// AppDelegate.cpp
#include "core/AppDelegate.h"
#include "base/CCDirector.h"
#include "platform/CCGLView.h"
#include "scenes/MenuScene.h"
USING_NS_CC;

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs(){
    GLContextAttrs a{8,8,8,8,24,8};
    GLView::setGLContextAttrs(a);
}

bool AppDelegate::applicationDidFinishLaunching(){
    auto* director = Director::getInstance();
    auto* glview = director->getOpenGLView();
    if(!glview){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        glview = GLViewImpl::createWithRect("MyGame", Rect(0,0,1280,720));
#else
        glview = GLViewImpl::create("MyGame");
#endif
        director->setOpenGLView(glview);
    }
    director->setAnimationInterval(1.0f/60.0f);
#if COCOS2D_DEBUG
    director->setDisplayStats(true);
#endif
    glview->setDesignResolutionSize(1280, 720, ResolutionPolicy::NO_BORDER);
    director->runWithScene(MenuScene::createScene());
    return true;
}
void AppDelegate::applicationDidEnterBackground(){ Director::getInstance()->stopAnimation(); }
void AppDelegate::applicationWillEnterForeground(){ Director::getInstance()->startAnimation(); }
