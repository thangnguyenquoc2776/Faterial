#include "game/Enemy.h"
#include "game/Player.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

using namespace cocos2d;

bool Enemy::init() {
    if (!Entity::init()) return false;

    // Tag gameplay mới
    setTagEx(static_cast<int>(phys::Tag::ENEMY));

    // Hình ảnh tạm (nếu bạn có frame thì đổi sau)
    _sprite = Sprite::create();
    _sprite->setTextureRect(Rect(0,0,42,42));
    _sprite->setColor(Color3B(200,70,70));
    addChild(_sprite);

    schedule([this](float dt){ updateEnemy(dt); }, "enemy.tick");
    return true;
}

void Enemy::enablePhysics(const Vec2& pos, const Size& bodySize) {
    _body = buildOrUpdateBody(bodySize);
    applyEnemyMasks();
    setPosition(pos);
}

PhysicsBody* Enemy::buildOrUpdateBody(const Size& sz) {
    Size box = sz;
    if (box.width < 1 || box.height < 1) box = Size(42,42);

    PhysicsBody* body = _body;
    if (!body) {
        body = PhysicsBody::createBox(box, PhysicsMaterial(0.2f, 0.0f, 0.4f));
        body->setDynamic(true);
        body->setRotationEnable(false);
        addComponent(body);
    } else {
        // rebuild shape
        auto shapes = body->getShapes();
        for (auto s : shapes) body->removeShape(s);
        auto s = PhysicsShapeBox::create(box, PhysicsMaterial(0.2f,0.0f,0.4f));
        body->addShape(s);
    }
    body->setLinearDamping(0.1f);
    body->setMass(1.0f);
    return body;
}

void Enemy::applyEnemyMasks() {
    if (!_body) return;
    _body->setCategoryBitmask(static_cast<int>(phys::CAT_ENEMY));
    // va chạm vật rắn + player + crate + gate
    _body->setCollisionBitmask(static_cast<int>(
        phys::CAT_WORLD | phys::CAT_PLAYER | phys::CAT_CRATE | phys::CAT_GATE));
    // nhận contact với mọi thứ để listener bắt sự kiện
    _body->setContactTestBitmask(static_cast<int>(phys::CAT_ALL));
}

void Enemy::setPatrol(const Vec2& a, const Vec2& b) {
    _pA = a; _pB = b;
    if (_pA.x > _pB.x) std::swap(_pA, _pB);
    // đặt vào giữa nếu chưa có vị trí
    if (getPosition().fuzzyEquals(Vec2::ZERO, 0.1f))
        setPosition((_pA + _pB) * 0.5f);
}

void Enemy::updateEnemy(float dt) {
    if (!_body) return;

    // Nếu có patrol, di chuyển qua lại theo X
    if (_pA != _pB) {
        float x = getPositionX();
        if (_dir > 0 && x >= _pB.x) _dir = -1;
        else if (_dir < 0 && x <= _pA.x) _dir = +1;

        Vec2 v = _body->getVelocity();
        v.x = _dir * _moveSpeed;
        // hạn tốc rơi
        v.y = std::max(v.y, -900.0f);
        _body->setVelocity(v);
    }
}

void Enemy::takeHit(int dmg) {
    _hp -= std::max(1, dmg);
    if (_hp <= 0) {
        // hiệu ứng nhỏ rồi biến mất
        runAction(Sequence::create(FadeOut::create(0.08f),
                                   CallFunc::create([this]{ removeFromParent(); }),
                                   nullptr));
    } else {
        // chớp đỏ
        if (_sprite) {
            _sprite->runAction(Sequence::create(TintTo::create(0.0f, 255,120,120),
                                                DelayTime::create(0.05f),
                                                TintTo::create(0.0f, 200,70,70),
                                                nullptr));
        }
    }
}
