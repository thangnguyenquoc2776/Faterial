#include "game/bosses/BossGolem.h"
#include "game/Player.h"                  // dùng Player* cho target
#include "game/objects/Star.h"            // rơi Star khi chết

#include "physics/PhysicsDefs.h"
#include "2d/CCDrawNode.h"
#include "physics/CCPhysicsBody.h"
#include "2d/CCActionInstant.h"
#include "base/CCDirector.h"
#include "physics/CCPhysicsBody.h"
#include "game/Player.h"
#include "physics/CCPhysicsShape.h"
#include "base/ccRandom.h"

#include <vector>
#include <algorithm>
#include <cmath>

using namespace cocos2d;

BossGolem* BossGolem::create() {
    auto p = new (std::nothrow) BossGolem();
    if (p && p->init()) { p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}

bool BossGolem::init() {
    if (!Enemy::init()) return false;

    setMaxHp(36);
    setMoveSpeed(70.f);
    _cd = 1.0f;

    if (_sprite) {
        _sprite->removeFromParent();
        _sprite = nullptr; // Reset con trỏ
    }

        _sprite = Sprite::create("sprites/boss/idle/idle_1.png");
        _sprite->setAnchorPoint(Vec2(0.5f, 0.5f)); // chân ở dưới
        _sprite->setPosition(Vec2(0.f, 150.f));
        _sprite->setScale(2.5f);
        addChild(_sprite, 1);

    auto size = _sprite->getContentSize() * _sprite->getScale();
    _body = PhysicsBody::createBox(size);
    setPhysicsBody(_body);

    // scheduleUpdate();
    // schedule([this](float){ updateEnemy(0); }, "enemy.tick");
    schedule([this](float dt){ this->_tickBoss(dt); }, "boss.ai");


    return true;
}



void BossGolem::takeHit(int dmg){
    if (_dead || _dying) return;

    const int hit = std::max(1, dmg);
    const int finalHp = _hp - hit;

    // hiệu ứng trúng đòn
    if (_sprite) {
        _sprite->runAction(Sequence::create(
            TintTo::create(0.0f, 255,150,150),
            DelayTime::create(0.06f),
            TintTo::create(0.0f, 120,170,255),
            nullptr
        ));
    }

    // chưa chết -> dùng logic Enemy thường (giảm máu + cập nhật thanh HP)
    if (finalHp > 0) { Enemy::takeHit(dmg); return; }

    // ====== chết: KHÔNG gọi Enemy::takeHit để né LootTable (coin/upgrade) ======
    _hp = 0;
    _dead  = true;
    _dying = true;

    // tắt AI và "đóng băng" body (không tháo shape ngay trong step hiện tại)
    unschedule("boss.ai");
    if (_body) {
        _body->setVelocity(Vec2::ZERO);
        _body->setContactTestBitmask(0);
        _body->setCollisionBitmask((int)phys::CAT_WORLD); // vẫn đứng yên nếu còn thấy
        _body->setEnabled(false);                         // tách khỏi physics world
    }

    // Hoãn 1 nhịp ngắn để thoát contact, rồi spawn Star + xóa boss
    this->runAction(Sequence::create(
        FadeOut::create(0.05f),
        DelayTime::create(0.02f),                 // QUAN TRỌNG: hoãn 1 frame
        CallFunc::create([this](){
            if (auto parent = getParent()) {
                auto star = Star::create();
                if (star) {
                    star->setPosition(getPosition() + Vec2(0, 24.f));
                    parent->addChild(star, 7);
                }
            }
            removeFromParent();
        }),
        nullptr
    ));
}

// =====================
// Helpers nội bộ
// =====================
namespace {
    // projectile mask cho ĐẠN ĐỊCH (đã định nghĩa CAT_ENEMY_PROJ trong PhysicsDefs)
    inline void setEnemyProjMasks(cocos2d::PhysicsBody* b) {
        b->setCategoryBitmask((int)phys::CAT_ENEMY_PROJ);
        b->setCollisionBitmask((int)(phys::CAT_WORLD | phys::CAT_PLAYER));
        b->setContactTestBitmask((int)(phys::CAT_PLAYER | phys::CAT_BULLET | phys::CAT_SENSOR));
    }

    // AOE vòng tròn sensor, tự huỷ nhanh
    Node* makeAoeRing(const Vec2& center, float radius, float lifeSec = 0.25f) {
        auto n = Node::create(); n->setPosition(center);
        n->setName("enemy_proj");

        auto dn = DrawNode::create();
        dn->drawCircle(Vec2::ZERO, radius, 0, 28, false, 1.8f, 1.8f, Color4F(1.f,0.8f,0.4f,0.7f));
        dn->drawSolidCircle(Vec2::ZERO, radius*0.85f, 0, 24, Color4F(1,0.8f,0.4f,0.10f));
        n->addChild(dn);

        auto body = PhysicsBody::createCircle(radius, PhysicsMaterial(0,0,0));
        body->setDynamic(false);
        for (auto s : body->getShapes()) s->setSensor(true);
        setEnemyProjMasks(body);
        n->addComponent(body);

        n->runAction(Sequence::create(
            DelayTime::create(std::max(0.05f, lifeSec)),
            CallFunc::create([n]{ n->removeFromParent(); }),
            nullptr));
        return n;
    }

    // Đạn thường: bay thẳng
    Node* makeBullet(const Vec2& origin, const Vec2& velocity, float lifeSec = 1.8f) {
        auto n = Node::create(); n->setPosition(origin);
        n->setName("enemy_proj");

        auto dn = DrawNode::create();
        dn->drawSolidCircle(Vec2::ZERO, 6.f, 0, 18, Color4F(0.95f,0.4f,0.2f,1));
        n->addChild(dn);

        auto body = PhysicsBody::createCircle(6.f, PhysicsMaterial(0,0,0));
        body->setDynamic(true);
        body->setGravityEnable(false);
        body->setRotationEnable(false);
        setEnemyProjMasks(body);
        n->addComponent(body);
        body->setVelocity(velocity);
        body->setLinearDamping(0.02f);

        n->runAction(Sequence::create(
            DelayTime::create(std::max(0.05f, lifeSec)),
            CallFunc::create([n]{ n->removeFromParent(); }),
            nullptr));
        return n;
    }

    // Đạn homing: bẻ lái dần
    Node* makeHoming(const Vec2& origin, Node* target,
                     float speed = 240.f, float turnRate = 6.0f, float lifeSec = 2.2f) {
        auto n = Node::create(); n->setPosition(origin);
        n->setName("enemy_proj");

        auto dn = DrawNode::create();
        dn->drawSolidCircle(Vec2::ZERO, 5.f, 0, 18, Color4F(0.9f,0.9f,1.f,1));
        n->addChild(dn);

        auto body = PhysicsBody::createCircle(5.f, PhysicsMaterial(0,0,0));
        body->setDynamic(true);
        body->setGravityEnable(false);
        body->setRotationEnable(false);
        setEnemyProjMasks(body);
        n->addComponent(body);

        n->schedule([n, target, speed, turnRate](float dt){
            auto b = n->getPhysicsBody(); 
            if (!b || !target) return;
            if (!target->getParent()) return;
            Vec2 from = n->getPosition(), to = target->getPosition();
            Vec2 dir  = (to - from).getNormalized();
            Vec2 desired = dir * speed;
            Vec2 v = b->getVelocity();
            float alpha = cocos2d::clampf(turnRate * dt, 0.f, 1.f);
            v = v * (1.f - alpha) + desired * alpha;
            if (v.lengthSquared() < speed*speed*0.25f) v = desired;
            b->setVelocity(v);
        }, "enemy.homing");

        n->runAction(Sequence::create(
            DelayTime::create(std::max(0.05f, lifeSec)),
            CallFunc::create([n]{ n->removeFromParent(); }),
            nullptr));
        return n;
    }
} // namespace (helpers)

// =====================
// AI patterns
// =====================
void BossGolem::_tickBoss(float dt){
    if (_dead || !_body) return;

    // KHÓA HOÀN TOÀN khi chưa aggro (bật ở mini cuối trong GameScene)
    if (!_aggroEnabled) {
        Vec2 v = _body->getVelocity();
        v.x = 0.f;
        v.y = std::max(v.y, -900.f);
        _body->setVelocity(v);
        return;
    }

    Player* p = (_target ? dynamic_cast<Player*>(_target) : nullptr);
    if (!p) return;

    Vec2 bp = getPosition();
    Vec2 pp = p->getPosition();
    float dist = bp.distance(pp);
    float dx   = pp.x - bp.x;

    const float AGGRO = 520.f;
    if (dist > AGGRO) {
        // di chuyển nhẹ khi chưa vào tầm
        Vec2 v = _body->getVelocity();
        v.x = (dx > 0 ? +1 : -1) * std::min(_moveSpeed * 0.6f, 80.f);
        v.y = std::max(v.y, -900.f);
        _body->setVelocity(v);
        playAnim("walk", 0.18f, 12, "boss");

        _cd -= dt * 0.5f;
        return;
    }

    _cd -= dt;
    if (_cd > 0.f) {
        Vec2 v = _body->getVelocity();
        v.x *= 0.9f;
        v.y = std::max(v.y, -900.f);
        _body->setVelocity(v);
        playAnim("walk", 0.18f, 12, "boss");
        if (_sprite) _sprite->setFlippedX(_dir > 0);


        return;
    }

    // Ưu tiên Radial; máu thấp thì hung hăng hơn
    int r = (_hp <= _maxHp/2) ? RandomHelper::random_int(0, 3)
                              : RandomHelper::random_int(0, 2);
    if (r <= 1) _radialBurst(pp);
    else        _blinkStrike(pp);

    _cd = 1.5f + RandomHelper::random_real(0.2f, 0.9f);
}

void BossGolem::_blinkStrike(const Vec2& playerPos){
    auto parent = getParent(); if (!parent) return;
    Vec2 bp = getPosition();
    int dir = (playerPos.x >= bp.x) ? +1 : -1;

    if (_sprite) _sprite->runAction(TintTo::create(0.0f, 200,240,255));
    runAction(Sequence::create(
        CallFunc::create([=](){ // BẮT ĐẦU hoạt ảnh ATTACK
            playAnim("attack", 0.18f, 15, "boss");
            if (_sprite) _sprite->setFlippedX(dir < 0); // Đổi hướng ở đây (hoặc trước đó)
        }),
        DelayTime::create(0.12f), // telegraph ngắn
        CallFunc::create([=](){
            // ... (phần code dịch chuyển, AOE) ...
            setPosition(bp + Vec2(dir * 180.f, 0));
            auto aoe = makeAoeRing(getPosition(), 90.f, 0.24f);
            if (aoe) parent->addChild(aoe, 7);
            if (_sprite) _sprite->runAction(TintTo::create(0.0f, 120,170,255));
            // Không cần setFlippedX ở đây nữa nếu đã set ở trên
        }),
        DelayTime::create(0.18f * 15), // Chờ hoạt ảnh Attack kết thúc (ví dụ 15 frame * 0.18s)
        CallFunc::create([=](){
            // Chuyển về idle/walk nếu cần
            // playAnim("walk", 0.18f, 12, "boss");
        }),
        nullptr
    ));
}

void BossGolem::_radialBurst(const Vec2& /*playerPos*/){
    auto parent = getParent(); if (!parent) return;

    const int N = RandomHelper::random_int(12, 16);
    const int K = RandomHelper::random_int(2, 4); // số viên homing
    const float S = 260.f;

    std::vector<int> homingIdx; homingIdx.reserve(K);
    for (int i=0; i<K; ++i) homingIdx.push_back(RandomHelper::random_int(0, N-1));

    float ang0 = RandomHelper::random_real(0.f, 3.1415926f);
    for (int i=0; i<N; ++i){
        float a = ang0 + i * (2*3.1415926f / N);
        Vec2 dir(std::cos(a), std::sin(a));
        bool isHoming = (std::find(homingIdx.begin(), homingIdx.end(), i) != homingIdx.end());

        Node* b = isHoming ? makeHoming(getPosition(), _target, 240.f, 6.0f, 2.2f)
                           : makeBullet(getPosition(), dir * S, 1.8f);
        if (b) parent->addChild(b, 6);
    }

    if (_sprite) _sprite->runAction(Sequence::create(
        TintTo::create(0.0f, 255,230,140),
        DelayTime::create(0.18f),
        TintTo::create(0.0f, 120,170,255),
        nullptr
    ));
}
