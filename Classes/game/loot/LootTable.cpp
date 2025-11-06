#include "game/loot/LootTable.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/objects/Upgrade.h"
#include "physics/CCPhysicsBody.h"
#include "physics/PhysicsDefs.h"   // <-- thêm dòng này


using namespace cocos2d;

static float frand(float a, float b){
    return RandomHelper::random_real(std::min(a,b), std::max(a,b));
}
static int irand(int a, int b){
    return RandomHelper::random_int(std::min(a,b), std::max(a,b));
}

const std::vector<DropSpec>& LootTable::defaultEnemyTable(){
    static std::vector<DropSpec> T = {
        { DropKind::COIN,    60.f, 1, 3 },
        { DropKind::UPGRADE, 25.f, 1, 1 },
        { DropKind::STAR,     5.f, 1, 1 },
        { DropKind::NONE,    10.f, 0, 0 }
    };
    return T;
}
const std::vector<DropSpec>& LootTable::defaultChestTable(){
    static std::vector<DropSpec> T = {
        { DropKind::COIN,    40.f, 3, 6 },
        { DropKind::UPGRADE, 45.f, 1, 2 },
        { DropKind::STAR,    10.f, 1, 1 },
        { DropKind::NONE,     5.f, 0, 0 }
    };
    return T;
}

DropKind LootTable::rollOne(const std::vector<DropSpec>& table, int& outCount){
    float sumW = 0.f;
    for (auto& e : table) sumW += e.weight;
    float r = frand(0.f, sumW), acc = 0.f;
    for (auto& e : table){
        acc += e.weight;
        if (r <= acc){
            outCount = (e.maxCount>=e.minCount)? irand(e.minCount, e.maxCount) : e.minCount;
            return e.kind;
        }
    }
    outCount = 0; return DropKind::NONE;
}

static void asSensorItem(Node* n){
    if (!n) return;
    auto* b = n->getPhysicsBody();
    if (!b){
        const float r = std::max(n->getContentSize().width, n->getContentSize().height) * 0.4f;
        b = PhysicsBody::createCircle(r, PhysicsMaterial(0,0,0));
        b->setDynamic(false);
        n->setPhysicsBody(b);
    }
    b->setGravityEnable(false);

    // mask ITEM sensor chuẩn theo PhysicsDefs
    b->setCategoryBitmask((int)phys::CAT_ITEM);
    b->setCollisionBitmask(0);                 // sensor-only
    b->setContactTestBitmask((int)phys::CAT_ALL);
    for (auto s : b->getShapes()) s->setSensor(true);
}


void LootTable::spawn(Node* root, const Vec2& p, DropKind k, int count){
    if (!root || k==DropKind::NONE || count<=0) return;
    const float spread = 28.f;

    for (int i=0;i<count;++i){
        Node* item=nullptr;
        switch (k){
            case DropKind::COIN:    item = Coin::create();    break;
            case DropKind::UPGRADE: item = Upgrade::create(); break;
            case DropKind::STAR:    item = Star::create();    break;
            default: break;
        }
        if (!item) continue;
        item->setPosition(p);
        root->addChild(item, 6);

        // sensor & hiệu ứng "bật" nhẹ
        asSensorItem(item);
        Vec2 dir = Vec2(frand(-1.f,1.f), 1.f).getNormalized();
        float dist = frand(spread*0.4f, spread);
        item->runAction(Sequence::create(
            MoveBy::create(0.25f, dir * dist),
            nullptr
        ));
    }
}

void LootTable::rollAndSpawn(Node* root, const Vec2& p, const std::vector<DropSpec>& table){
    int cnt=0; DropKind k = rollOne(table, cnt);
    spawn(root, p, k, cnt);
}

void LootTable::dropAt(Node* root, const Vec2& p, int count) {
    if (!root) return;
    count = std::max(1, count);
    for (int i = 0; i < count; ++i) {
        rollAndSpawn(root, p, defaultEnemyTable());
    }
}
