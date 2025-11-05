#include "game/Player.h"

#include "2d/CCSprite.h"
#include "2d/CCDrawNode.h"
#include "2d/CCActionInstant.h"
#include "2d/CCActionInterval.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

using namespace cocos2d;

Player* Player::create(const std::string& frame) {
    auto* p = new (std::nothrow) Player();
    if (p && p->initWithFrame(frame)) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool Player::initWithFrame(const std::string& frame) {
    if (!Entity::init()) return false;

    if (!frame.empty()) {
        _sprite  = Sprite::create(frame);
        _boxSize = _sprite->getContentSize();
    } else {
        _boxSize = Size(32.f, 48.f);
        _sprite  = Sprite::create();
        auto dn  = DrawNode::create();
        // vẽ theo TÂM (CENTER)
        dn->drawSolidRect(Vec2(-_boxSize.width*0.5f, -_boxSize.height*0.5f),
                          Vec2( _boxSize.width*0.5f,  _boxSize.height*0.5f),
                          Color4F(0.90f,0.90f,1.f,1.f));
        _sprite->addChild(dn);
        _sprite->setContentSize(_boxSize);
    }

    // CENTER-anchored sprite để trùng tâm collider
    _sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    _sprite->setPosition(Vec2::ZERO);
    addChild(_sprite);

    setTagEx(phys::Tag::PLAYER);
    schedule([this](float dt){ updateMove(dt); }, "player.move.tick");
    return true;
}

void Player::enablePhysics(const Vec2& spawnCenter, const Size& bodySize) {
    setPosition(spawnCenter);         // Node = CENTER
    _body = buildOrUpdateBody(bodySize);
    applyPlayerMasks();
}

PhysicsBody* Player::buildOrUpdateBody(const Size& bodySize) {
    Size sz = bodySize;
    if (sz.equals(Size::ZERO)) {
        sz = _sprite ? _sprite->getContentSize() : Size(32,48);
        if (sz.width < 1 || sz.height < 1) sz = Size(32,48);
    }
    _boxSize = sz;

    if (!_body) {
        _body = PhysicsBody::create();
        _body->setDynamic(true);
        _body->setRotationEnable(false);
        addComponent(_body);
    } else {
        for (auto s : _body->getShapes()) _body->removeShape(s);
    }

    // Hộp chính TRÙNG TÂM (CENTER) — KHÔNG offset +h/2
    auto mainBox = PhysicsShapeBox::create(
        sz, PhysicsMaterial(0.1f, 0.0f, 0.5f),
        Vec2::ZERO
    );
    _body->addShape(mainBox);

    // Sensor chân — đặt NGAY DƯỚI đáy hộp
    const float footH   = std::max(4.0f, sz.height * 0.08f);
    const Size  footSize(sz.width * 0.55f, footH);
    const Vec2  footOffset(0, -sz.height * 0.5f - footH * 0.5f);
    auto foot = PhysicsShapeBox::create(footSize, PhysicsMaterial(0,0,0), footOffset);
    foot->setSensor(true);
    foot->setTag(static_cast<int>(phys::ShapeTag::FOOT));
    _body->addShape(foot);

    _body->setMass(1.0f);
    _body->setLinearDamping(0.1f);
    return _body;
}

void Player::applyPlayerMasks() {
    if (!_body) return;
    const phys::Mask collide = phys::CAT_WORLD | phys::CAT_ENEMY |
                               phys::CAT_SOLID | phys::CAT_CRATE | phys::CAT_GATE;
    const phys::Mask contact = phys::CAT_WORLD | phys::CAT_ENEMY |
                               phys::CAT_ITEM  | phys::CAT_BULLET | phys::CAT_SENSOR;
    phys::setMasks(_body, phys::CAT_PLAYER, collide, contact);
}

void Player::setMoveDir(const Vec2& dir) {
    _moveDir = dir;
    if (_moveDir.x >  0.05f) _facing = +1;
    else if (_moveDir.x < -0.05f) _facing = -1;
}

void Player::applyVelocity(float) {
    if (!_body) return;
    const float targetVx = _moveDir.x * moveSpeed;
    const Vec2  v        = _body->getVelocity();
    const float factor   = (_footContacts > 0) ? 1.0f : airControl;
    const float blend    = cocos2d::clampf(factor, 0.0f, 1.0f);
    const float newVx    = v.x + (targetVx - v.x) * blend;
    const float maxFall  = -900.0f;
    const float vy       = std::max(v.y, maxFall);
    _body->setVelocity(Vec2(newVx, vy));
}

void Player::updateMove(float dt) { applyVelocity(dt); }

void Player::jump() {
    if (!_body || _footContacts <= 0) return;
    Vec2 v = _body->getVelocity();
    if (v.y < 0) v.y = 0;
    _body->setVelocity(v);
    _body->applyImpulse(Vec2(0, jumpImpulse));
}

void Player::incFoot(int delta) {
    _footContacts += delta;
    if (_footContacts < 0) _footContacts = 0;
}

void Player::hurt(int dmg, const Vec2& knock) {
    if (_invincible) return;
    hp -= std::max(1, dmg);
    if (hp < 0) hp = 0;
    if (_body && (knock.x != 0 || knock.y != 0)) _body->applyImpulse(knock);
    _invincible = true;
    runAction(Sequence::create(
        DelayTime::create(0.6f),
        CallFunc::create([this]{ _invincible = false; }),
        nullptr
    ));
}
