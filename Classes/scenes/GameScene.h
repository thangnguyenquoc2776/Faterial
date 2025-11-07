#pragma once
#include "cocos2d.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "game/map/LevelBuilder.h"
#include "2d/CCParallaxNode.h"

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
    // world / view
    cocos2d::PhysicsWorld* _world = nullptr;
    cocos2d::Size _vs{};
    cocos2d::Vec2 _origin{};

    // UI
    cocos2d::Camera* _uiCam = nullptr;
    HUDLayer*        _hud   = nullptr;
    cocos2d::Label*  _overlay = nullptr;

    // actors
    Player* _player = nullptr;
    cocos2d::Vector<Enemy*> _enemies;

    // level state
    float _groundTop = 0.f;
    int   _segment = 0, _segmentCount = 1;
    float _segmentWidth = 0.f;
    float _camL = 0.f, _camR = 0.f;

    int  _score=0, _lives=3, _starsHave=0, _starsNeed=5;
    bool _gameOver=false, _gameWin=false;
    bool _bossAggroOn=false; // chỉ bật khi vào mini cuối

    // listeners
    cocos2d::EventListenerKeyboard*       _kb      = nullptr;
    cocos2d::EventListenerPhysicsContact* _contact = nullptr;

    // UI helpers
    void buildUICamera();
    void buildHUD();
    void _showOverlay(const std::string& text);

    // input
    void _bindInput();

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

    // stars by segment + gates + end portal
    std::vector<int> _starsSeg;
    std::vector<cocos2d::Node*> _locks;
    cocos2d::Node* _endPortal = nullptr;
};
