#pragma once
#include "cocos2d.h"
#include "ui/HUDLayer.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "game/map/LevelBuilder.h"
#include "physics/PhysicsDefs.h"

USING_NS_CC;

class GameScene : public Layer {
public:
    static Scene* createScene();
    CREATE_FUNC(GameScene);

    bool init() override;
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;

private:
    // ---- WORLD/PHYSICS ----
    Scene*        _scene = nullptr;
    PhysicsWorld* _world = nullptr; // <-- chỉ lấy trong onEnter
    Size _vs;
    Vec2 _origin;

    // ---- PLAYER/ENEMY ----
    Player* _player = nullptr;
    Vector<Enemy*> _enemies;

    // ---- CAMERA/ZONES (map liên tiếp) ----
    int   _segment       = 0;
    int   _segmentCount  = 1;
    float _segmentWidth  = 0.f;
    float _groundTop     = 0.f;  // y-top mặt đất chuẩn để check rơi
    float _camL = 0.f, _camR = 0.f;

    // ---- UI/HUD ----
    Camera*   _uiCam   = nullptr;
    HUDLayer* _hud     = nullptr;
    Label*    _overlay = nullptr;

    int  _score     = 0;
    int  _lives     = 3;
    int  _starsHave = 0;
    int  _starsNeed = 5;
    bool _gameOver  = false;
    bool _gameWin   = false;

    // ---- INPUT/LISTENERS ----
    EventListenerKeyboard*       _kb      = nullptr;
    EventListenerPhysicsContact* _contact = nullptr;

private:
    // ===== UI =====
    void buildUICamera();
    void buildHUD();

    // ===== INPUT =====
    void _bindInput();
    void _doShoot();
    void _doSlash();

    // ===== CONTACT =====
    bool _onContactBegin(PhysicsContact& c);
    void _onContactSeparate(PhysicsContact& c);

    // ===== HUD/STATE =====
    void _setLives(int v);
    void _addScore(int v);
    void _setStars(int have, int need);
    void _checkWin();
    void _showOverlay(const std::string& text);

    // ===== FLOW =====
    void _restartLevel();
    void _returnMenu();
};
