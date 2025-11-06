#pragma once
#include "cocos2d.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "game/map/LevelBuilder.h"

class HUDLayer;

class GameScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(GameScene);

    bool init() override;
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;

    void setPhysicsWorld(cocos2d::PhysicsWorld* w) { _world = w; }

private:
    cocos2d::PhysicsWorld* _world = nullptr;
    cocos2d::Size _vs{};
    cocos2d::Vec2 _origin{};

    cocos2d::Camera* _uiCam = nullptr;
    HUDLayer*        _hud   = nullptr;
    cocos2d::Label*  _overlay = nullptr;

    Player* _player = nullptr;
    cocos2d::Vector<Enemy*> _enemies;

    float _groundTop = 0.f;
    int   _segment = 0, _segmentCount = 1;
    float _segmentWidth = 0.f;
    float _camL = 0.f, _camR = 0.f;

    int  _score=0, _lives=3, _starsHave=0, _starsNeed=5;
    bool _gameOver=false, _gameWin=false;

    cocos2d::EventListenerKeyboard*       _kb      = nullptr;
    cocos2d::EventListenerPhysicsContact* _contact = nullptr;

    // UI
    void buildUICamera();
    void buildHUD();
    void _showOverlay(const std::string& text);

    // input
    void _bindInput();
    void _doShoot();
    void _doSlash();

    // contact
    bool _onContactBegin(cocos2d::PhysicsContact& c);
    void _onContactSeparate(cocos2d::PhysicsContact& c);

    // HUD/state
    void _setLives(int v);
    void _addScore(int v);
    void _setStars(int have, int need);
    void _checkWin();
    void _restartLevel();
    void _returnMenu();
};
