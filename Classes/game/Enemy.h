#pragma once
#include "cocos2d.h"
#include "game/Entity.h"
#include "physics/PhysicsDefs.h"
#include "game/Entity.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

class Player;

class Enemy : public Entity {
public:
    CREATE_FUNC(Enemy);
    bool init() override;
    void enablePhysics(const cocos2d::Vec2& pos, const cocos2d::Size& bodySize = cocos2d::Size(42, 42));

    void setPatrol(const cocos2d::Vec2& a, const cocos2d::Vec2& b);

    // Target + Aggro
    void setTarget(Player* p)             { _target = p; }
    void setAggroEnabled(bool on)         { _aggroEnabled = on; }
    void setAggroRange(float r)           { _aggroRange = std::max(0.f, r); }
    void setChaseSpeed(float v)           { _chaseSpeed = std::max(10.f, v); }

    virtual void takeHit(int dmg);
    virtual void updateEnemy(float dt);   // có chase theo aggro

    void setMoveSpeed(float v) { _moveSpeed = v; }
    void setMaxHp(int v) { _maxHp = _hp = std::max(1, v); _updateHpBar(); }

protected:
    cocos2d::PhysicsBody* buildOrUpdateBody(const cocos2d::Size& sz);
    void applyEnemyMasks();

    void _updateHpBar();
    void _dropLoot();

protected:
    cocos2d::Sprite*      _sprite  = nullptr;
    cocos2d::PhysicsBody* _body    = nullptr;
    cocos2d::DrawNode*    _hpbar   = nullptr;
    Player*               _target  = nullptr;

    std::string _currentAnim;
    
    //!PLAY ANIMATION
    void playAnim(const std::string& name, float delay, int frames, const std::string& className);
    // !helper to build frames either from SpriteFrameCache (.plist) or from individual png files
    virtual cocos2d::Vector<cocos2d::SpriteFrame*> buildFrames(const std::string& animName, int frameCount, const std::string& className);
    cocos2d::Vec2 _pA, _pB;
    int   _dir = +1;
    float _moveSpeed = 90.f;      // tuần tra
    float _chaseSpeed = 140.f;    // đuổi
    float _aggroRange = 220.f;    // tầm phát hiện (theo X, kèm giới hạn Y)
    bool  _aggroEnabled = true;

    // Stat
    int _hp = 2, _maxHp = 2;
    bool _dead = false;
};
