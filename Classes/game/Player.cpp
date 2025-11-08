// Classes/game/Player.cpp  — phiên bản có SFX bắn/chém + jump/land/hurt (fallback đuôi)
#include "game/Player.h"

#include "2d/CCDrawNode.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

#include "GameScene.h"
#include "game/weapon/Bullet.h"
#include "game/weapon/Slash.h"

// Âm thanh
#include "audio/Sound.h"                        
#include "audio/include/AudioEngine.h"          // fallback SFX cho jump/land/hurt
#include "platform/CCFileUtils.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace cocos2d;

// // ==============================
// // SFX helpers (fallback by stem)
// // ==============================
// namespace {
//     using AE = cocos2d::AudioEngine;

//     // Cho phép chỉ truyền "gốc tên" (stem), tự tìm .wav/.mp3/.ogg nếu tồn tại
//     inline std::string _pickAudio(const char* stem) {
//         static const char* exts[] = { ".wav", ".mp3", ".ogg" };
//         auto* fu = FileUtils::getInstance();
//         for (auto* ext : exts) {
//             std::string p = std::string(stem) + ext;
//             if (fu->isFileExist(p)) return p;
//         }
//         CCLOG("SND missing: %s.(wav|mp3|ogg) not found", stem);
//         return {};
//     }

//     inline void _sfxOptional(const char* stem, float vol = 0.9f) {
//         if (!stem) return;
//         std::string f = _pickAudio(stem);
//         if (!f.empty()) AE::play2d(f, false, vol);
//     }

//     // Tên gốc (stem) cho các SFX thêm mới:
//     constexpr const char* SFX_JUMP = "audio/sfx_jump";
//     constexpr const char* SFX_LAND = "audio/sfx_land";
//     constexpr const char* SFX_HURT = "audio/sfx_hurt";
// }

// ==============================

static inline float approach(float cur, float target, float delta) {
    if (cur < target) return std::min(target, cur + delta);
    if (cur > target) return std::max(target, cur - delta);
    return cur;
}

bool Player::init() {
    if (!Entity::init()) return false;
    CCLOG("Player::init");
    this->setName("player");


    _sprite = Sprite::create("sprites/player/idle/idle_1.png");
    _sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    _sprite->setPositionY(_colSize.height * 0.65f);
    _sprite->setScale(1.4f);
    addChild(_sprite, 1);

    scheduleUpdate();
    return true;
}

void Player::refreshVisual() {
    // (tùy chọn) debug collider
}

void Player::enablePhysics(const Vec2& feetPos, const Size& bodySize) {
    if (bodySize.width > 0 && bodySize.height > 0) _colSize = bodySize;
    setPosition(feetPos);

    _body = buildOrUpdateBody(_colSize);
    applyPlayerMasks();

    _body->setEnabled(true);          // đúng chính tả cocos2d-x
    _body->setGravityEnable(true);
    _body->setDynamic(true);
    _body->setVelocity(Vec2::ZERO);
}

PhysicsBody* Player::buildOrUpdateBody(const Size& bodySize) {
    Size sz = bodySize;
    sz.width  = std::max(18.f, sz.width  * 0.90f);
    sz.height = std::max(30.f, sz.height * 1.5f);

    if (!_body) {
        _body = PhysicsBody::create();
        _body->setDynamic(true);
        _body->setRotationEnable(false);
        addComponent(_body);
    } else {
        auto shapes = _body->getShapes();
        for (auto s : shapes) _body->removeShape(s);
    }

    // main box (trọng tâm nằm giữa thân)
    auto mainBox = PhysicsShapeBox::create(
        sz, PhysicsMaterial(0.1f, 0.0f, 0.5f),
        Vec2(0, +sz.height * 0.5f)
    );
    _body->addShape(mainBox);

    // chân (sensor) — để đếm tiếp đất
    const float footH = std::max(4.0f, sz.height * 0.08f);
    const Size  footSize(sz.width * 0.60f, footH);
    const Vec2  footOffset(0, -footH * 0.5f);
    auto foot = PhysicsShapeBox::create(footSize, PhysicsMaterial(0,0,0), footOffset);
    foot->setSensor(true);
    foot->setTag((int)phys::ShapeTag::FOOT);
    _body->addShape(foot);

    _body->setMass(1.0f);
    _body->setLinearDamping(0.1f);
    return _body;
}

void Player::applyPlayerMasks() {
    if (!_body) return;

    // va chạm cứng
    const phys::Mask collide =
        phys::CAT_WORLD | phys::CAT_ENEMY | phys::CAT_SOLID |
        phys::CAT_GATE  | phys::CAT_CRATE;

    // contact (thêm ENEMY_PROJ để ăn đạn địch)
    const phys::Mask contact =
        phys::CAT_WORLD | phys::CAT_ENEMY | phys::CAT_ENEMY_PROJ |
        phys::CAT_ITEM  | phys::CAT_BULLET | phys::CAT_SENSOR;

    phys::setMasks(_body, phys::CAT_PLAYER, collide, contact);
}

void Player::setMoveDir(const Vec2& dir) {
    _moveDir = dir;
    if (_moveDir.x < 0) _facing = -1;
    else if (_moveDir.x > 0) _facing = +1;
}

void Player::jump() {
    if (!_body) return;
    const float m = _body->getMass();
    const float impulse = _jumpImpulse * m;

    if (_footContacts > 0) {
        _body->applyImpulse(Vec2(0, impulse));
        snd::sfxJump();       // 🔊 nhảy từ đất
        return;
    }
    if (_airJumpsMax > 0 && _airJumpsUsed < _airJumpsMax) {
        _airJumpsUsed++;
        _body->setVelocity(Vec2(_body->getVelocity().x, 0));
        _body->applyImpulse(Vec2(0, impulse * 0.92f));
        snd::sfxJump();          // 🔊 nhảy trên không
    }
}

void Player::incFoot(int delta) {
    const int prev = _footContacts;
    _footContacts = std::max(0, _footContacts + delta);
    if (prev==0 && _footContacts>0) {
        _airJumpsUsed = 0;            // chạm đất -> reset
        // _sfxOptional(SFX_LAND);       // 🔊 chạm đất
    }
}

void Player::heal(int v) { _hp = std::min(_maxHp, _hp + std::max(0, v)); }
void Player::restoreFullHP() { _hp = _maxHp; }

void Player::hurt(int dmg) {
    if (invincible() || !_body) return;
    _hp = std::max(0, _hp - std::max(1, dmg));
    _invincibleT = 0.8f;
    // _sfxOptional(SFX_HURT);           // 🔊 bị thương

    const float dir = (_facing > 0 ? -1.f : +1.f);
    _body->applyImpulse(Vec2(200.f * dir, 260.f));
}

void Player::doShoot(){
    if (_shooting) return;
    _shooting = true;

    int dir = facing();
    runAction(Sequence::create(
        DelayTime::create(0.35f),
        CallFunc::create([this, dir]() {
            int count = 1 + std::max(0, _extraBullets);
            float spread = 0.08f;
            for (int i=0;i<count;i++){
                float offY = (i - (count-1)*0.5f) * 8.0f;
                Vec2 origin = getPosition() + Vec2(dir * 18.f, halfH() * 1.5f + offY);
                Vec2 vel(700.f * dir, (i - (count-1)*0.5f) * 120.f * spread);
                auto b = Bullet::create(origin, vel, 1.5f);
                if (getParent() && b) getParent()->addChild(b, 6);
            }
            snd::sfxShoot();          // 🔊 bắn
        }),
        DelayTime::create(0.36f),
        CallFunc::create([this]() { _shooting = false; }),
        nullptr
    ));
}

void Player::doSlash(){
    if (_attacking) return;
    _attacking = true;
    const int dir = facing();

    runAction(Sequence::create(
        DelayTime::create(0.20f),
        CallFunc::create([this, dir]() {
            Vec2 origin = getPosition() + Vec2(dir*28.f, halfH()*0.1f);
            float angle = (dir > 0) ? 0.0f : 3.14159265f;
            auto s = Slash::create(origin, angle, _slashRange, 0.12f);
            if (auto p=getParent()) p->addChild(s, 6);
            snd::sfxSlash();          // 🔊 chém
        }),
        DelayTime::create(0.30f),
        CallFunc::create([this]{ _attacking = false; }),
        nullptr));
}

void Player::applyUpgrade(int type, float duration){
    float factor = 1.f;
    switch(type){
        case 0: factor = 1.25f; _moveSpeed   *= factor; break; // SPEED
        case 1: factor = 1.15f; _jumpImpulse *= factor; break; // JUMP
        case 2: _atkBonus += 1; break;                           // DAMAGE (+1)
        case 3: _extraBullets += 1; break;                       // BULLET
        case 4: /* RANGE tăng tầm, xử lý trong Slash.create bằng _slashRange */ break;
        case 5: _airJumpsMax = std::max(_airJumpsMax, 1); break; // DOUBLEJUMP
        default: break;
    }
    _buffs.push_back({type, std::max(0.1f, duration), factor, -1});
}

cocos2d::Vector<cocos2d::SpriteFrame*> Player::buildFrames(const std::string& animName, int frameCount) {
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= frameCount; ++i) {
        std::string path = StringUtils::format("sprites/player/%s/%s_%d.png", animName.c_str(), animName.c_str(), i);
        auto tex = Director::getInstance()->getTextureCache()->addImage(path);
        if (!tex) break;
        Rect rect(0, 0, tex->getPixelsWide(), tex->getPixelsHigh());
        frames.pushBack(SpriteFrame::create(path, rect));
    }
    return frames;
}

void Player::playAnim(const std::string& animName, float delay, int frameCount) {
    if (_currentAnim == animName) return;
    auto frames = buildFrames(animName, frameCount);
    if (frames.empty()) return;

    _currentAnim = animName;
    auto animation = Animation::createWithSpriteFrames(frames, delay);
    auto act = RepeatForever::create(Animate::create(animation));
    if (_sprite) { _sprite->stopAllActions(); _sprite->runAction(act); }
}

void Player::update(float dt) {
    if (!_body) return;

    // invuln blink
    if (_invincibleT > 0.f) {
        _invincibleT -= dt;
        const bool blink = ((int)std::floor(_invincibleT * 20.f)) % 2 == 0;
        if (_sprite) _sprite->setOpacity(blink ? 140 : 255);
    } else if (_sprite) {
        _sprite->setOpacity(255);
    }

    const bool grounded = (_footContacts > 0);
    const float targetVx = _moveDir.x * _moveSpeed;

    Vec2 v = _body->getVelocity();
    const float accel = grounded ? _accelGround : _accelAir;

    if (grounded && std::abs(_moveDir.x) < 1e-3f) v.x = approach(v.x, 0.f, accel * dt);
    else v.x = approach(v.x, targetVx, accel * dt);

    v.y = std::max(v.y, -_maxFall);
    _body->setVelocity(v);

    bool runningNow = (grounded && std::abs(_moveDir.x) > 1e-3f);
    //! PLAY RUNNING SFX
    if (runningNow && !_wasRunning) snd::sfxRun(); 
    // !STOP RUNNING SFX
    if (!runningNow && _wasRunning) {
        snd::stopSfxRun();
    }
    _wasRunning = runningNow;


    // chọn anim
    if (isDead()) nextAnim = "die";
    else if (_attacking) nextAnim = "attack";
    else if (_shooting)  nextAnim = "shoot";
    else if (!grounded)  nextAnim = "jump";
    else if (std::abs(_moveDir.x) > 1e-3f) nextAnim = "run";
    else nextAnim = "idle";

    if (_currentAnim != nextAnim){
        if (nextAnim == "die")         playAnim("die",    0.12f, 6);
        else if (nextAnim == "jump")   playAnim("jump",   0.23f, 4);
        else if (nextAnim == "run")    playAnim("run",    0.06f, 8);
        else if (nextAnim == "attack") playAnim("attack", 0.06f, 6);
        else if (nextAnim == "shoot")  playAnim("shoot",  0.08f, 6);
        else                           playAnim("idle",   0.12f, 8);
    }

    // lật sprite vẫn giữ scale gốc
    if (_sprite) {
        float sx = std::fabs(_sprite->getScaleX());
        if (sx < 0.01f) sx = 1.4f; // nếu trước đó bị set về 0/1, lấy lại mặc định
        _sprite->setScaleX(_facing * sx);
    }

    // đếm buff & revert
    for (int i=(int)_buffs.size()-1; i>=0; --i){
        _buffs[i].remain -= dt;
        if (_buffs[i].remain <= 0.f){
            int type = _buffs[i].type; float factor = _buffs[i].factor;
            switch(type){
                case 0: _moveSpeed   /= factor; break;
                case 1: _jumpImpulse /= factor; break;
                case 2: _atkBonus    = std::max(0, _atkBonus-1); break;
                case 3: _extraBullets= std::max(0, _extraBullets-1); break;
                case 5: _airJumpsMax = 0; _airJumpsUsed = 0; break;
                default: break;
            }
            _buffs.erase(_buffs.begin()+i);
        }
    }
}
