#include "game/Enemy.h"
#include "game/Player.h"

#include "game/objects/Coin.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Chest.h"
#include "game/loot/LootTable.h"

#include "physics/PhysicsDefs.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

#include "2d/CCDrawNode.h"
#include "base/ccRandom.h"

USING_NS_CC;

static inline float sgn(float x) { return (x > 0.f) ? 1.f : (x < 0.f ? -1.f : 0.f); }

bool Enemy::init() {
    if (!Entity::init()) return false;
    setTagEx((int)phys::Tag::ENEMY);

    _sprite = Sprite::create();
    _sprite->setTextureRect(Rect(0, 0, 42, 42));
    _sprite->setColor(Color3B(200, 70, 70));
    addChild(_sprite);

    _hpbar = DrawNode::create();
    addChild(_hpbar, 2);

    schedule([this](float dt) { updateEnemy(dt); }, "enemy.tick");
    return true;
}

void Enemy::enablePhysics(const Vec2& pos, const Size& bodySize) {
    _body = buildOrUpdateBody(bodySize);
    applyEnemyMasks();
    setPosition(pos);
    _updateHpBar();
}

PhysicsBody* Enemy::buildOrUpdateBody(const Size& szIn) {
    Size box = (szIn.width < 1 || szIn.height < 1) ? Size(42, 42) : szIn;
    PhysicsBody* body = _body;

    if (!body) {
        body = PhysicsBody::createBox(box, PhysicsMaterial(0.2f, 0.0f, 0.4f));
        body->setDynamic(true);
        body->setRotationEnable(false);
        addComponent(body);
    } else {
        auto shapes = body->getShapes();
        for (auto s : shapes) body->removeShape(s);
        body->addShape(PhysicsShapeBox::create(box, PhysicsMaterial(0.2f, 0.0f, 0.4f)));
    }

    body->setLinearDamping(0.1f);
    body->setMass(1.0f);
    return body;
}

void Enemy::applyEnemyMasks() {
    if (!_body) return;
    _body->setCategoryBitmask((int)phys::CAT_ENEMY);
    _body->setCollisionBitmask((int)(phys::CAT_WORLD | phys::CAT_PLAYER | phys::CAT_CRATE | phys::CAT_GATE));
    _body->setContactTestBitmask((int)phys::CAT_ALL);
}

void Enemy::setPatrol(const Vec2& a, const Vec2& b) {
    _pA = a; _pB = b;
    if (_pA.x > _pB.x) std::swap(_pA, _pB);
    if (getPosition().fuzzyEquals(Vec2::ZERO, 0.1f))
        setPosition((_pA + _pB) * 0.5f);
}

void Enemy::updateEnemy(float) {
    if (!_body) return;

    Vec2 v = _body->getVelocity();

    // Đuổi khi bật aggro + trong tầm (giới hạn lệch Y để không “hút xuyên tầng”)
    bool chasing = false;
    if (_aggroEnabled && _target) {
        float dx = _target->getPositionX() - getPositionX();
        float dy = _target->getPositionY() - getPositionY();
        if (std::abs(dx) <= _aggroRange && std::abs(dy) <= 160.f) {
            _dir = (dx >= 0 ? +1 : -1);
            v.x = _dir * _chaseSpeed;
            chasing = true;
        }
    }

    // Không đuổi -> tuần tra giữa 2 mốc
    if (!chasing && _pA != _pB) {
        float x = getPositionX();
        if (_dir > 0 && x >= _pB.x) _dir = -1;
        else if (_dir < 0 && x <= _pA.x) _dir = +1;
        v.x = _dir * _moveSpeed;
    }

    v.y = std::max(v.y, -900.0f);
    _body->setVelocity(v);
}

void Enemy::_updateHpBar() {
    _hpbar->clear();
    const float w = 46.f, h = 4.f;
    const float t = std::max(0.f, std::min(1.f, _hp / (float)_maxHp));
    Vec2 base(-w * 0.5f, 28.f);
    _hpbar->drawSolidRect(base, base + Vec2(w, h), Color4F(0, 0, 0, 0.6f));
    _hpbar->drawSolidRect(base + Vec2(1, 1), base + Vec2(1 + (w - 2) * t, h - 1),
                          Color4F(0.9f, 0.2f, 0.2f, 0.95f));
}

// Rơi đồ: đẩy về LootTable (đã loại STAR)
void Enemy::_dropLoot() {
    if (auto parent = getParent())
        LootTable::dropAt(parent, getPosition());
}

void Enemy::takeHit(int dmg) {
    if (_dead) return;

    _hp -= std::max(1, dmg);
    _updateHpBar();

    if (_hp <= 0) {
        _dead = true;

        // Ngắt tương tác vật lý rồi rơi đồ đúng 1 lần
        scheduleOnce([this](float) {
            if (_body) {
                _body->setVelocity(Vec2::ZERO);
                _body->setDynamic(false);
                _body->setCategoryBitmask(0);
                _body->setCollisionBitmask(0);
                _body->setContactTestBitmask(0);
            }

            _dropLoot(); // <- duy nhất chỗ này, KHÔNG Star

            runAction(Sequence::create(
                FadeOut::create(0.08f),
                CallFunc::create([this] { removeFromParent(); }),
                nullptr
            ));
        }, 0.0f, "enemy_die_defer");
        return;
    }

    if (_sprite) {
        _sprite->runAction(Sequence::create(
            TintTo::create(0.0f, 255, 120, 120),
            DelayTime::create(0.05f),
            TintTo::create(0.0f, 200, 70, 70),
            nullptr
        ));
    }
}
