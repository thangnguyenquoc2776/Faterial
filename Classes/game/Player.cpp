#include "game/Player.h"
#include "2d/CCDrawNode.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

using namespace cocos2d;

static inline float approach(float cur, float target, float delta) {
    if (cur < target) return std::min(target, cur + delta);
    if (cur > target) return std::max(target, cur - delta);
    return cur;
}

bool Player::init() {
    if (!Entity::init()) return false;

    // GFX: hình chữ nhật đơn giản (đặt theo feet = y=0)
    _gfx = DrawNode::create();
    addChild(_gfx, 1);
    refreshVisual();

    scheduleUpdate();
    return true;
}

void Player::refreshVisual() {
    if (!_gfx) return;
    _gfx->clear();
    const float w = _colSize.width;
    const float h = _colSize.height;
    // vì feet = y=0 nên thân phải vẽ lệch lên nửa chiều cao
    _gfx->drawSolidRect(Vec2(-w*0.5f, 0.f), Vec2(w*0.5f, h),
                        Color4F(0.90f, 0.90f, 0.98f, 1.f));
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
    // làm hitbox “dễ chịu” hơn một chút
    sz.width  = std::max(18.f, sz.width  * 0.90f);
    sz.height = std::max(30.f, sz.height * 0.98f);

    if (!_body) {
        _body = PhysicsBody::create();
        _body->setDynamic(true);
        _body->setRotationEnable(false); // không cho lộn đầu
        addComponent(_body);
    } else {
        // xoá mọi shape cũ để rebuild
        auto shapes = _body->getShapes();
        for (auto s : shapes) _body->removeShape(s);
    }

    // BOX chính: đẩy lên nửa chiều cao để feet = y=0
    auto mainBox = PhysicsShapeBox::create(
        sz, PhysicsMaterial(0.1f, 0.0f, 0.5f),
        Vec2(0, +sz.height * 0.5f)
    );
    // Không setTag(HITBOX) để khỏi lệ thuộc enum không tồn tại
    _body->addShape(mainBox);

    // FOOT sensor: mỏng, đặt ngay dưới đáy feet một chút
    const float footH = std::max(4.0f, sz.height * 0.08f);
    const Size  footSize(sz.width * 0.60f, footH);
    const Vec2  footOffset(0, -footH * 0.5f); // cắt mỏng xuyên qua đường tiếp xúc

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

void Player::hurt(int /*dmg*/) {
    if (invincible() || !_body) return;
    _invincibleT = 0.8f;

    // Hất nhẹ về phía ngược lại
    const float dir = (_facing > 0 ? -1.f : +1.f);
    _body->applyImpulse(Vec2(200.f * dir, 260.f));
}

void Player::update(float dt) {
    if (!_body) return;

    // Bất tử nháy màu
    if (_invincibleT > 0.f) {
        _invincibleT -= dt;
        const bool blink = ((int)std::floor(_invincibleT * 20.f)) % 2 == 0;
        if (_gfx) _gfx->setOpacity(blink ? 140 : 255);
    } else {
        if (_gfx) _gfx->setOpacity(255);
    }

    const bool grounded = (_footContacts > 0);
    const float targetVx = _moveDir.x * _moveSpeed;

    Vec2 v = _body->getVelocity();
    const float accel = grounded ? _accelGround : _accelAir;

    // nếu không bấm trái/phải và đang trên đất → phanh về 0
    if (grounded && std::abs(_moveDir.x) < 1e-3f)
        v.x = approach(v.x, 0.f, accel * dt);
    else
        v.x = approach(v.x, targetVx, accel * dt);

    // hạn chế rơi quá nhanh
    v.y = std::max(v.y, -_maxFall);

    _body->setVelocity(v);
}
