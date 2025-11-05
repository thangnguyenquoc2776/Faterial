#pragma once
#include "cocos2d.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "game/map/LevelBuilder.h"

class HUDLayer;

class GameScene : public cocos2d::Layer {
public:
    // Tạo scene vật lý đúng chuẩn (KHÔNG crash khi vào)
    static cocos2d::Scene* createScene();
    CREATE_FUNC(GameScene);

    bool init() override;
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;

private:
    // World/Scene
    cocos2d::Scene*        _scene = nullptr;
    cocos2d::PhysicsWorld* _world = nullptr;

    // Viewport
    cocos2d::Size _vs{};
    cocos2d::Vec2 _origin{};

    // Gameplay
    Player* _player = nullptr;
    cocos2d::Vector<Enemy*> _enemies;

    float _groundTop   = 0.f;
    int   _segment     = 0;
    int   _segmentCount= 1;
    float _segmentWidth= 0.f;
    float _camL = 0.f, _camR = 0.f;

    // UI
    cocos2d::Camera*   _uiCam   = nullptr;
    HUDLayer*          _hud     = nullptr;
    cocos2d::Label*    _overlay = nullptr;

    // State
    int  _score=0, _lives=3, _starsHave=0, _starsNeed=5;
    bool _gameOver=false, _gameWin=false;

    // Listeners
    cocos2d::EventListenerKeyboard*       _kb      = nullptr;
    cocos2d::EventListenerPhysicsContact* _contact = nullptr;

private:
    // Helpers
    void buildUICamera();
    void buildHUD();
    void _bindInput();

    // Combat helpers
    void _doShoot();
    void _doSlash();

    // Contacts
    bool _onContactBegin(cocos2d::PhysicsContact& c);
    void _onContactSeparate(cocos2d::PhysicsContact& c);

    // HUD/state helpers
    void _setLives(int v);
    void _addScore(int v);
    void _setStars(int have, int need);
    void _checkWin();
    void _showOverlay(const std::string& text);
    void _restartLevel();
    void _returnMenu();
};
