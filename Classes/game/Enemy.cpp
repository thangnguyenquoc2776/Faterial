#include "game/Enemy.h"
#include "game/Player.h"
#include "game/objects/Coin.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Chest.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include "base/ccRandom.h"
#include "game/loot/LootTable.h"
USING_NS_CC;

bool Enemy::init() {
    if (!Entity::init()) return false;
    setTagEx((int)phys::Tag::ENEMY);

    _sprite = Sprite::create();
    _sprite->setTextureRect(Rect(0,0,42,42));
    _sprite->setColor(Color3B(200,70,70));
    addChild(_sprite);

    _hpbar = DrawNode::create();
    addChild(_hpbar, 2);

    schedule([this](float dt){ updateEnemy(dt); }, "enemy.tick");
    return true;
}

void Enemy::enablePhysics(const Vec2& pos, const Size& bodySize) {
    _body = buildOrUpdateBody(bodySize);
    applyEnemyMasks();
    setPosition(pos);
    _updateHpBar();
}

PhysicsBody* Enemy::buildOrUpdateBody(const Size& sz) {
    Size box = (sz.width<1||sz.height<1)? Size(42,42) : sz;
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

void Enemy::_updateHpBar(){
    _hpbar->clear();
    const float w = 46.f, h = 4.f;
    const float t = std::max(0.f, std::min(1.f, _hp/(float)_maxHp));
    Vec2 base(-w*0.5f, 28.f);
    _hpbar->drawSolidRect(base, base+Vec2(w,h), Color4F(0,0,0,0.6f));
    _hpbar->drawSolidRect(base+Vec2(1,1), base+Vec2(1+(w-2)*t, h-1), Color4F(0.9f,0.2f,0.2f,0.95f));
}

void Enemy::_dropLoot(){
    float r = RandomHelper::random_real(0.f,1.f);
    Node* parent = getParent(); if (!parent) return;

    if (r < 0.05f) { // chest 5%
        auto ch = Chest::create();
        ch->setPosition(getPosition());
        parent->addChild(ch, 5);
    } else if (r < 0.30f) { // upgrade 25%
        auto u = Upgrade::createRandom();
        if (u){ u->setPosition(getPosition()+Vec2(0,10)); parent->addChild(u,5); }
    } else { // coins
        int n = RandomHelper::random_int(1,3);
        for (int i=0;i<n;++i){
            auto c = Coin::create();
            c->setPosition(getPosition()+Vec2(RandomHelper::random_real(-12.f,12.f), 6));
            parent->addChild(c,5);
        }
    }
}

void Enemy::takeHit(int dmg) {
    if (_dead) return;

    _hp -= std::max(1, dmg);
    _updateHpBar();

    if (_hp <= 0) {
        _dead = true;

        // HOÃN mọi thao tác phá/loot/bitmask sang frame kế tiếp
        this->scheduleOnce([this](float){
            // tắt va chạm triệt để, không đụng setEnable (không tồn tại)
            if (_body) {
                _body->setVelocity(cocos2d::Vec2::ZERO);
                _body->setDynamic(false);
                _body->setCategoryBitmask(0);
                _body->setCollisionBitmask(0);
                _body->setContactTestBitmask(0);
            }
            if (auto parent = getParent()) {
                LootTable::dropAt(parent, getPosition()); // coin/upgrade random
            }
            this->runAction(cocos2d::Sequence::create(
                cocos2d::FadeOut::create(0.08f),
                cocos2d::CallFunc::create([this]{ removeFromParent(); }),
                nullptr
            ));
        }, 0.0f, "enemy_die_defer");

        return;
    }

    if (_sprite) {
        _sprite->runAction(cocos2d::Sequence::create(
            cocos2d::TintTo::create(0.0f, 255,120,120),
            cocos2d::DelayTime::create(0.05f),
            cocos2d::TintTo::create(0.0f, 200,70,70),
            nullptr
        ));
    }
}

