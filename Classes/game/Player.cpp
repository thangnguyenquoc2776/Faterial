#include "game/Player.h"
#include "2d/CCDrawNode.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include <algorithm>
#include <cmath>
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"
#include "GameScene.h" // nếu bạn cần dùng getParent() hoặc dynamic_cast

#include "game/weapon/Bullet.h"
#include "game/weapon/Slash.h"

using namespace cocos2d;

static inline float approach(float cur, float target, float delta) {
    if (cur < target) return std::min(target, cur + delta);
    if (cur > target) return std::max(target, cur - delta);
    return cur;
}

bool Player::init() {
    if (!Entity::init()) return false;
    CCLOG("Player::init");

    // _gfx = DrawNode::create();
    // addChild(_gfx, 1);
    // refreshVisual();

    _sprite = Sprite::create("sprites/player/idle/idle_1.png"); // frame đầu tiên
    _sprite->setAnchorPoint(Vec2(0.5f, 0.5f)); // chân ở dưới
    _sprite->setPositionY(_colSize.height * 0.5f); // dời lên nửa collider
    _sprite->setScale(1.4f); // tùy chỉnh kích cỡ hiển thị
    addChild(_sprite, 1);


    scheduleUpdate();
    return true;
}

void Player::refreshVisual() {
    // if (!_gfx) return;
    // _gfx->clear();
    // const float w = _colSize.width;
    // const float h = _colSize.height;
    // _gfx->drawSolidRect(Vec2(-w*0.5f, 0.f), Vec2(w*0.5f, h),
    //                     Color4F(0.90f, 0.90f, 0.98f, 1.f));
}

void Player::enablePhysics(const Vec2& feetPos, const Size& bodySize) {
    if (bodySize.width > 0 && bodySize.height > 0) _colSize = bodySize;
    setPosition(feetPos);

    _body = buildOrUpdateBody(_colSize);
    applyPlayerMasks();

    _body->setEnabled(true);
    _body->setGravityEnable(true);
    _body->setDynamic(true);
    _body->setVelocity(Vec2::ZERO);
}

PhysicsBody* Player::buildOrUpdateBody(const Size& bodySize) {
    Size sz = bodySize;
    sz.width  = std::max(18.f, sz.width  * 0.90f);
    //! config lai height 1.5 cho bang sprite
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

    auto mainBox = PhysicsShapeBox::create(
        sz, PhysicsMaterial(0.1f, 0.0f, 0.5f),
        Vec2(0, +sz.height * 0.5f)
    );
    _body->addShape(mainBox);

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

    const phys::Mask collide =
        phys::CAT_WORLD | phys::CAT_ENEMY | phys::CAT_SOLID |
        phys::CAT_GATE  | phys::CAT_CRATE;
    const phys::Mask contact =
        phys::CAT_WORLD | phys::CAT_ENEMY | phys::CAT_ITEM   |
        phys::CAT_BULLET| phys::CAT_SENSOR;

    phys::setMasks(_body, phys::CAT_PLAYER, collide, contact);
}

void Player::setMoveDir(const Vec2& dir) {
    _moveDir = dir;
    if (_moveDir.x < 0) _facing = -1;
    else if (_moveDir.x > 0) _facing = +1;
}

void Player::jump() {
    if (!_body) return;
    if (_footContacts <= 0) return;
    const float m = _body->getMass();
    _body->applyImpulse(Vec2(0, _jumpImpulse * m));
}

void Player::incFoot(int delta) {
    _footContacts += delta;
    if (_footContacts < 0) _footContacts = 0;
}

void Player::heal(int v) {
    _hp = std::min(_maxHp, _hp + std::max(0, v));
}
void Player::restoreFullHP() { _hp = _maxHp; }

void Player::hurt(int dmg) {
    if (invincible() || !_body) return;
    _hp = std::max(0, _hp - std::max(1, dmg));
    _invincibleT = 0.8f;

    const float dir = (_facing > 0 ? -1.f : +1.f);
    _body->applyImpulse(Vec2(200.f * dir, 260.f));
}

// Bắn / chém giữ nguyên
void Player::doShoot(){
    if (_shooting) return; // tránh spam
    _shooting = true;

    int dir = facing();


    runAction(Sequence::create(
        DelayTime::create(0.35f), // delay 0.35 giây
        CallFunc::create([this, dir]() {
            Vec2 origin = getPosition() + Vec2(dir * 18.f, halfH() * 0.25f);
            auto b = Bullet::create(origin, Vec2(700.f * dir, 0.f), 1.5f);
            if (getParent() && b)
                getParent()->addChild(b, 6);
        }),
        DelayTime::create(0.36f), // cooldown trước khi có thể bắn lại
        CallFunc::create([this]() { _shooting = false; }),
        nullptr
    ));

}

void Player::doSlash(){
     if (_attacking ) return; // tránh spam
    _attacking = true;

    int dir = facing();
    
    // Delay 0.25s trước khi spawn hitbox (trúng sau khi vung)
    runAction(Sequence::create(
        DelayTime::create(0.25f),
        CallFunc::create([this, dir]() {
            Vec2 origin = getPosition() + Vec2(dir * 28.f, halfH() * 0.1f);
            float angle = (dir > 0) ? 0.0f : M_PI;
            auto s = Slash::create(origin, angle, 60.f, 0.12f);
            if (getParent()) getParent()->addChild(s, 6);
        }),
        // Cho nghỉ 0.36s rồi mới cho phép tấn công tiếp
        DelayTime::create(0.36f),
        CallFunc::create([this]() {
            _attacking = false;
        }),
        nullptr
    ));
}


Vector<SpriteFrame*> Player::buildFrames(const std::string& animName, int frameCount) {

    Vector<SpriteFrame*> frames;
    for (int i = 1; i < frameCount+1; ++i) {
        std::string path = StringUtils::format("sprites/player/%s/%s_%d.png", animName.c_str(), animName.c_str(), i);
        auto tex = Director::getInstance()->getTextureCache()->addImage(path);
        if (!tex) break; // không tìm thấy file => dừng
        Rect rect(0, 0, tex->getPixelsWide(), tex->getPixelsHigh());
        frames.pushBack(SpriteFrame::create(path, rect));
    }
    return frames;

}

void Player::playAnim(const std::string& animName, float delay, int frameCount) {
    CCLOG("Play anim: %s", animName.c_str());
    if (_currentAnim == animName) return;

    auto frames = buildFrames(animName, frameCount);
    if (frames.empty()) {
        CCLOG("Không tìm thấy frame cho anim: %s", animName.c_str());
        return;
    }
    _currentAnim = animName;
    auto animation = Animation::createWithSpriteFrames(frames, delay);
    auto act = RepeatForever::create(Animate::create(animation));

    if (_sprite) {
        _sprite->stopAllActions();
        _sprite->runAction(act);
    }
}

void Player::update(float dt) {
    if (!_body) return;

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

    if (grounded && std::abs(_moveDir.x) < 1e-3f)
        v.x = approach(v.x, 0.f, accel * dt);
    else
        v.x = approach(v.x, targetVx, accel * dt);

    v.y = std::max(v.y, -_maxFall);
    _body->setVelocity(v);

    // !--- animation select ---

    if (isDead()) nextAnim = "die";
    else if (_attacking) nextAnim = "attack";
    else if (_shooting) nextAnim = "shoot";
    else if (!grounded) nextAnim = "jump";
    else if (std::abs(_moveDir.x) > 1e-3f) nextAnim = "run";
    
    else nextAnim = "idle";

    if (_currentAnim != nextAnim)
    {
        if (nextAnim == "die") playAnim("die", 0.12f, 6);
        else if (nextAnim == "jump") playAnim("jump", 0.23f, 4);
        else if (nextAnim == "run") playAnim("run", 0.06f, 8);
        else if (nextAnim == "attack") playAnim("attack", 0.06f, 6);
        else if (nextAnim == "shoot") playAnim("shoot", 0.08f, 6);
        else playAnim("idle", 0.12f, 8);
    }

    if (_sprite) _sprite->setScaleX(_facing);
}
