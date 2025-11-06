#include "game/Enemy.h"
#include "game/Player.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include "2d/CCDrawNode.h"

using namespace cocos2d;

static inline void drawMiniHp(DrawNode* dn, int cur, int max) {
    if (!dn) return;
    dn->clear();
    const float W=30.f, H=4.f, pad=1.f;
    dn->drawSolidRect(Vec2(-W*0.5f, 10), Vec2(W*0.5f, 10+H), Color4F(0,0,0,0.55f));
    float ratio = (max>0)? (float)cur/(float)max : 0.f;
    float ww = (W-2*pad) * ratio;
    dn->drawSolidRect(Vec2(-W*0.5f+pad, 10+pad), Vec2(-W*0.5f+pad+ww, 10+H-pad),
                      Color4F(0.25f,0.95f,0.35f,1));
}

bool Enemy::init() {
    if (!Entity::init()) return false;

    setTagEx(static_cast<int>(phys::Tag::ENEMY));

    _sprite = Sprite::create();
    _sprite->setTextureRect(Rect(0,0,42,42));
    _sprite->setColor(Color3B(200,70,70));
    addChild(_sprite);

    _hpMini = DrawNode::create();
    addChild(_hpMini, 5);
    drawMiniHp(_hpMini, _hp, _maxHp);

    schedule([this](float){ updateEnemy(0); }, "enemy.tick");
    return true;
}

void Enemy::enablePhysics(const Vec2& pos, const Size& bodySize) {
    _body = buildOrUpdateBody(bodySize);
    applyEnemyMasks();
    setPosition(pos);
}

PhysicsBody* Enemy::buildOrUpdateBody(const Size& sz) {
    Size box = sz.width<1||sz.height<1 ? Size(42,42) : sz;

    PhysicsBody* body = _body;
    if (!body) {
        body = PhysicsBody::createBox(box, PhysicsMaterial(0.2f, 0.0f, 0.4f));
        body->setDynamic(true);
        body->setRotationEnable(false);
        addComponent(body);
    } else {
        auto shapes = body->getShapes();
        for (auto s : shapes) body->removeShape(s);
        body->addShape(PhysicsShapeBox::create(box, PhysicsMaterial(0.2f,0.0f,0.4f)));
    }
    body->setLinearDamping(0.1f);
    body->setMass(1.0f);
    return body;
}

void Enemy::applyEnemyMasks() {
    if (!_body) return;
    _body->setCategoryBitmask(static_cast<int>(phys::CAT_ENEMY));
    _body->setCollisionBitmask(static_cast<int>(
        phys::CAT_WORLD | phys::CAT_PLAYER | phys::CAT_CRATE | phys::CAT_GATE));
    _body->setContactTestBitmask(static_cast<int>(phys::CAT_ALL));
}

void Enemy::setPatrol(const Vec2& a, const Vec2& b) {
    _pA = a; _pB = b;
    if (_pA.x > _pB.x) std::swap(_pA, _pB);
    if (getPosition().fuzzyEquals(Vec2::ZERO, 0.1f))
        setPosition((_pA + _pB) * 0.5f);
}

void Enemy::updateEnemy(float) {
    if (!_body) return;
    if (_pA != _pB) {
        float x = getPositionX();
        if (_dir > 0 && x >= _pB.x) _dir = -1;
        else if (_dir < 0 && x <= _pA.x) _dir = +1;
        Vec2 v = _body->getVelocity();
        v.x = _dir * _moveSpeed;
        v.y = std::max(v.y, -900.0f);
        _body->setVelocity(v);
    }
}

void Enemy::takeHit(int dmg) {
    _hp -= std::max(1, dmg);
    drawMiniHp(_hpMini, _hp, _maxHp);
    if (_hp <= 0) {
        runAction(Sequence::create(FadeOut::create(0.08f),
                                   CallFunc::create([this]{ removeFromParent(); }),
                                   nullptr));
    } else if (_sprite) {
        _sprite->runAction(Sequence::create(TintTo::create(0.0f, 255,120,120),
                                            DelayTime::create(0.05f),
                                            TintTo::create(0.0f, 200,70,70),
                                            nullptr));
    }
}
