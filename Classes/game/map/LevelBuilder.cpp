#include "game/map/LevelBuilder.h"
#include "physics/PhysicsDefs.h"
#include "game/objects/Coin.h"
#include "game/objects/Chest.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Star.h"

USING_NS_CC;
namespace levels {

// helper: tạo nền/phẳng rắn
static Node* makeSolid(Node* root, const Rect& r, const Color4F& col) {
    auto dn = DrawNode::create();
    dn->drawSolidRect(r.origin, r.origin + r.size, col);
    root->addChild(dn, 1);

    auto n = Node::create();
    n->setPosition(Vec2::ZERO);
    auto body = PhysicsBody::createBox(r.size, PhysicsMaterial(0.1f,0,0.6f), r.origin + r.size*0.5f);
    body->setDynamic(false);
    body->setCategoryBitmask((int)phys::CAT_WORLD);
    body->setCollisionBitmask((int)(phys::CAT_ALL & ~phys::CAT_BULLET));
    body->setContactTestBitmask((int)phys::CAT_ALL);
    n->addComponent(body);
    root->addChild(n, 2);
    return n;
}

BuildResult buildLevel1(Node* root, const Size& vs, const Vec2& origin) {
    BuildResult L;
    L.segments = 5;
    L.segmentWidth = vs.width;
    L.groundTop = origin.y + 36.f;
    L.playerSpawn = origin + Vec2(80.f, L.groundTop + 40.f);

    // Ground dài 5 đoạn
    makeSolid(root, Rect(origin.x, origin.y, vs.width*L.segments, 24.f), Color4F(0.2f,0.85f,0.2f,1));

    // Một vài platform mỗi mini
    for (int s=0; s<L.segments; ++s) {
        float baseX = origin.x + s*vs.width;
        // platform giữa
        makeSolid(root, Rect(baseX + vs.width*0.40f, L.groundTop+180.f, 220.f, 16.f),
                  Color4F(0.55f,0.58f,0.95f,1));

        // enemy tuần tra
        auto e = Enemy::create();
        e->enablePhysics(Vec2(baseX + vs.width*0.55f, L.groundTop+200.f), Size(38,38));
        e->setPatrol(Vec2(baseX + vs.width*0.44f, L.groundTop+200.f),
                     Vec2(baseX + vs.width*0.68f, L.groundTop+200.f));
        root->addChild(e, 4);
        L.enemies.pushBack(e);

        // coin rải
        for (int i=0;i<3;++i){
            auto c = Coin::create();
            c->setPosition(Vec2(baseX + 140 + i*36, L.groundTop+220));
            root->addChild(c, 5);
        }

        // 1 upgrade nổi
        auto u = Upgrade::createRandom();
        if (u){ u->setPosition(Vec2(baseX + vs.width*0.22f, L.groundTop+120)); root->addChild(u,5); }

        // 1 chest
        auto ch = Chest::create();
        ch->setPosition(Vec2(baseX + vs.width*0.82f, L.groundTop+40));
        root->addChild(ch,5);

        // star (mục tiêu mini)
        auto star = Star::create();
        star->setPosition(Vec2(baseX + vs.width*0.50f, L.groundTop+260));
        root->addChild(star,6);
    }

    return L;
}

} // namespace levels
