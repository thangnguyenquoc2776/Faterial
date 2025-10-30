#pragma once
#include "cocos2d.h"
class Player; class HUDLayer; class Gate; class BossGolem;

class GameScene : public cocos2d::Scene {
public:
    CREATE_FUNC(GameScene);
    static cocos2d::Scene* createScene();
    bool init() override;
    void onEnter() override;
    void update(float dt) override;                 // << NEW

private:
    // world
    cocos2d::Node* _world=nullptr;
    float _worldW=0.f, _worldH=0.f;

    // actors & ui
    Player*   _player=nullptr;
    HUDLayer* _hud=nullptr;
    Gate*     _gateFinal=nullptr;
    BossGolem* _boss=nullptr;

    // gameplay
    int _score=0, _lives=3;
    int _stars=0, _starsNeed=5;
    int _zoneIdx=1, _zoneTot=5;
    int _bgmId=-1;

    // setup helpers
    void _setupWorld();
    void _buildZones();
    void _spawnPlayer(const cocos2d::Vec2& p);
    void _bindInput();
    void _followCamera(float);
    void _attachTargetsToEnemies();                 // << NEW

    // collisions
    bool _onContactBegin(cocos2d::PhysicsContact& c);
    bool _match(cocos2d::Node* n, uint32_t cat, int tag = 0);

    // utilities
    void _addScore(int s);
    void _pickupStar();
    void _tryOpenFinalGate();
    void _spawnCoinAt(const cocos2d::Vec2& p, int n);
    void _playSfx(const std::string& rel, float vol=1.f);
    void _playBgm(const std::string& rel, bool loop, float vol);
};
