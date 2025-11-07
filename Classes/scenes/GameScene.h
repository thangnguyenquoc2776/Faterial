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
    // ------------- scene roots -------------
    cocos2d::Node*   _bgRoot    = nullptr; // parallax/bg (nếu có)
    cocos2d::Node*   _worldRoot = nullptr; // platforms, items, enemies, gates...
    cocos2d::Node*   _fxRoot    = nullptr; // text/hiệu ứng bám world
    cocos2d::Node*   _uiRoot    = nullptr; // HUD/PAUSE... bám màn hình
    cocos2d::Camera* _uiCam     = nullptr;

    // physics
    cocos2d::PhysicsWorld* _world = nullptr;

    // view
    cocos2d::Size _vs{};
    cocos2d::Vec2 _origin{};

    // UI
    HUDLayer*       _hud      = nullptr;
    cocos2d::Label* _overlay  = nullptr;   // overlay UI (screen-space)

    // actors
    Player* _player = nullptr;
    cocos2d::Node* _playerAnchor = nullptr; // gắn nhãn bay theo player
    cocos2d::Vector<Enemy*> _enemies;

    // level state
    float _groundTop = 0.f;
    int   _segment = 0, _segmentCount = 1;
    float _segmentWidth = 0.f;
    float _worldWidth   = 0.f; // bề rộng toàn bản đồ

    int  _score=0, _lives=3, _starsHave=0, _starsNeed=5;
    bool _gameOver=false, _gameWin=false;
    bool _bossAggroOn=false;

    // listeners
    cocos2d::EventListenerKeyboard*       _kb      = nullptr;
    cocos2d::EventListenerPhysicsContact* _contact = nullptr;

    // ---------- build / helpers ----------
    void _buildSceneRoots();
    void buildUICamera();
    void buildHUD();
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

    // gate/portal
    void _buildTallGates();
    int  _miniIndexAt(float x) const;

    // overlay
    void _showScreenOverlay(const std::string& text);                   // dính màn hình
    void _showWorldTextAt(const std::string& text, const cocos2d::Vec2& worldPos); // bám world
    float _cameraCenterX() const;

    // add helpers (set camera masks đúng flag)
    void _addToWorld(cocos2d::Node* n, int z=0);
    void _addToFX   (cocos2d::Node* n, int z=0);
    void _addToUI   (cocos2d::Node* n, int z=0);
};
