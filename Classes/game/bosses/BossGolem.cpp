#include "game/bosses/BossGolem.h"
#include "game/objects/Star.h"
#include "game/objects/Chest.h"
#include "game/loot/LootTable.h"
#include "physics/CCPhysicsBody.h"
USING_NS_CC;

BossGolem* BossGolem::create(){
    auto p = new(std::nothrow) BossGolem();
    if (p && p->init()) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}

bool BossGolem::init(){
    if (!Enemy::init()) return false;

    setMaxHp(30);
    setMoveSpeed(70.f);

    if (_sprite) {
        _sprite->setTextureRect(Rect(0,0,72,72));
        _sprite->setColor(Color3B(120,170,255));
        _sprite->setScale(1.2f);
    }
    enablePhysics(getPosition(), Size(72,72));
    return true;
}

void BossGolem::takeHit(int dmg){
    if (_dead) return;

    _hp -= std::max(1, dmg);
    _updateHpBar();

    if (_hp <= 0){
        _dead = true;

        this->scheduleOnce([this](float){
            if (_body) {
                _body->setVelocity(Vec2::ZERO);
                _body->setDynamic(false);
                _body->setCategoryBitmask(0);
                _body->setCollisionBitmask(0);
                _body->setContactTestBitmask(0);
            }
            if (auto parent = getParent()) {
                // BẮT BUỘC: sao để hoàn màn
                if (auto star = Star::create()){
                    star->setPosition(getPosition() + Vec2(0, 48));
                    parent->addChild(star, 6);
                }
                // Tuỳ chọn:
                // LootTable::dropAt(parent, getPosition(), 2); // thêm coin/upgrade
                // if (auto chest = Chest::create()){ chest->setPosition(getPosition()+Vec2(36,0)); parent->addChild(chest, 6); }
            }

            runAction(Sequence::create(
                FadeOut::create(0.12f),
                CallFunc::create([this]{ removeFromParent(); }),
                nullptr
            ));
        }, 0.0f, "boss_die_defer");

        return;
    }

    if (_sprite){
        _sprite->runAction(Sequence::create(
            TintTo::create(0.0f, 150,200,255),
            DelayTime::create(0.05f),
            TintTo::create(0.0f, 120,170,255),
            nullptr
        ));
    }
}

