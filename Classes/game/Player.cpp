#include "game/Player.h"

#include <algorithm>
#include "2d/CCSprite.h"
#include "2d/CCDrawNode.h"
#include "2d/CCActionInstant.h"
#include "2d/CCActionInterval.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

using namespace cocos2d;

// ----------------------------------------------------
// Factory / init
// ----------------------------------------------------
Player* Player::create(const std::string& frame) {
    auto* p = new (std::nothrow) Player();
    if (p && p->initWithFrame(frame)) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool Player::init() {
    // CREATE_FUNC hoặc các nơi gọi init() mặc định -> không frame
    return initWithFrame(std::string());
}

bool Player::initWithFrame(const std::string& frame) {
    if (!Entity::init()) return false;

    // Nếu có frame thì dùng Sprite, không thì vẽ placeholder 32x48
    if (!frame.empty()) {
        _sprite = Sprite::create(frame);
        if (!_sprite) {
            // fallback placeholder nếu không load được frame
            auto dn = DrawNode::create();
            const float w = 32.f, h = 48.f;
            dn->drawSolidRect(Vec2(-w/2, -h/2), Vec2(w/2, h/2), Color4F(0.9f,0.9f,1.f,1.f));
            _sprite = Sprite::create();
            _sprite->addChild(dn);
            _sprite->setContentSize(Size(w, h));
        }
    } else {
        auto dn = DrawNode::create();
        const float w = 32.f, h = 48.f;
        dn->drawSolidRect(Vec2(-w/2, -h/2), Vec2(w/2, h/2), Color4F(0.9f,0.9f,1.f,1.f));
        _sprite = Sprite::create();
        _sprite->addChild(dn);
        _sprite->setContentSize(Size(w, h));
    }

    _sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    addChild(_sprite);

    // Tag gameplay
    setTagEx(phys::Tag::PLAYER);

    // Tick áp vận tốc
    schedule([this](float dt){ updateMove(dt); }, "player.move.tick");
    return true;
}

// ----------------------------------------------------
// Physics
// ----------------------------------------------------
void Player::enablePhysics(const Vec2& pos, const Size& bodySize) {
    setPosition(pos);
    enableBody(true, bodySize);
}

void Player::enableBody(bool on, const Size& bodySize) {
    if (!on) {
        if (_body) { removeComponent(_body); _body = nullptr; }
        return;
    }
    _body = buildOrUpdateBody(bodySize);
    applyPlayerMasks();
}

PhysicsBody* Player::buildOrUpdateBody(const Size& wantSize) {
    Size sz = wantSize;
    if (sz.equals(Size::ZERO)) {
        sz = _sprite ? _sprite->getContentSize() : Size(32, 48);
        if (sz.width < 1 || sz.height < 1) sz = Size(32, 48);
    }

    PhysicsBody* body = _body;
    if (!body) {
        body = PhysicsBody::createBox(sz, PhysicsMaterial(0.1f, 0.0f, 0.5f));
        body->setDynamic(true);
        body->setRotationEnable(false);
        addComponent(body);
    } else {
        // Cập nhật collider chính: xoá hết shape cũ và tạo lại
        auto shapes = body->getShapes();
        for (auto s : shapes) body->removeShape(s);
        body->addShape(PhysicsShapeBox::create(sz, PhysicsMaterial(0.1f, 0.0f, 0.5f)));
    }

    // Sensor chân (FOOT) — tạo shape với offset ngay từ factory
    const float footH = std::max(4.0f, sz.height * 0.08f);
    const Size  footSize(sz.width * 0.5f, footH);
    const Vec2  footOffset(0, -sz.height * 0.5f + footH * 0.5f);

    auto footShape = PhysicsShapeBox::create(footSize, PhysicsMaterial(0,0,0), footOffset);
    footShape->setSensor(true);
    footShape->setTag(static_cast<int>(phys::ShapeTag::FOOT));
    body->addShape(footShape); // chữ ký 1 tham số trong cocos2d-x 3.x

    body->setMass(1.0f);
    body->setLinearDamping(0.1f);
    return body;
}

void Player::applyPlayerMasks() {
    if (!_body) return;

    // Collide: các thứ chặn cứng người chơi
    const phys::Mask collide = phys::CAT_WORLD | phys::CAT_ENEMY |
                               phys::CAT_SOLID | phys::CAT_CRATE | phys::CAT_GATE;

    // Contact test: muốn nhận callback khi chạm
    const phys::Mask contact = phys::CAT_WORLD | phys::CAT_ENEMY |
                               phys::CAT_ITEM  | phys::CAT_BULLET | phys::CAT_SENSOR;

    phys::setMasks(_body, phys::CAT_PLAYER, collide, contact);
}

// ----------------------------------------------------
// Control / Move
// ----------------------------------------------------
void Player::setMoveDir(const Vec2& dir) {
    _moveDir = dir;
    if (_moveDir.x >  0.05f) _facing = +1;
    else if (_moveDir.x < -0.05f) _facing = -1;
}

void Player::applyVelocity(float /*dt*/) {
    if (!_body) return;

    const float targetVx = _moveDir.x * moveSpeed;
    const Vec2  v        = _body->getVelocity();

    const float factor = (_footContacts > 0) ? 1.0f : airControl;
    const float blend  = cocos2d::clampf(factor, 0.0f, 1.0f);
    const float newVx  = v.x + (targetVx - v.x) * blend;

    const float maxFall = -900.0f;
    const float vy      = std::max(v.y, maxFall);

    _body->setVelocity(Vec2(newVx, vy));
}

void Player::updateMove(float dt) {
    applyVelocity(dt);
}

void Player::jump() {
    if (!_body || _footContacts <= 0) return;

    // Giảm đà rơi cho cú nhảy mượt
    Vec2 v = _body->getVelocity();
    if (v.y < 0) v.y = 0;
    _body->setVelocity(v);

    _body->applyImpulse(Vec2(0, jumpImpulse));
}

void Player::incFoot(int delta) {
    _footContacts += delta;
    if (_footContacts < 0) _footContacts = 0;
}

// ----------------------------------------------------
// Combat / HP
// ----------------------------------------------------
void Player::hurt(int dmg) {
    // Overload không knockback
    hurt(dmg, Vec2::ZERO);
}

void Player::hurt(int dmg, const Vec2& knockImpulse) {
    if (_invincible) return;

    hp -= std::max(1, dmg);
    if (hp < 0) hp = 0;

    if (_body && (knockImpulse.x != 0 || knockImpulse.y != 0)) {
        _body->applyImpulse(knockImpulse);
    }

    _invincible = true;
    runAction(Sequence::create(
        DelayTime::create(0.6f),
        CallFunc::create([this]{ _invincible = false; }),
        nullptr
    ));
}
