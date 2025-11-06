#include "game/map/LevelBuilder.h"
#include "physics/PhysicsDefs.h"

#include "2d/CCDrawNode.h"
#include "2d/CCLayer.h"

#include "game/Enemy.h"
#include "game/enemies/Goomba.h"
#include "game/enemies/Spiker.h"
#include "game/bosses/BossGolem.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"

using namespace cocos2d;

namespace {

// NỀN/PLATFORM “top-anchored”: node đặt tại (cx, topY - h/2)
static Node* addPlatformTop(Node* root, float cx, float topY, float w, float h, const Color4F& c) {
    auto n = Node::create();
    n->setPosition(Vec2(cx, topY - h * 0.5f));

    auto* body = PhysicsBody::createBox(Size(w, h), PhysicsMaterial(0.15f, 0.0f, 0.3f));
    body->setDynamic(false);

    // KHÔNG dùng MASK_ALL (vì header của bạn không có) → set qua helper trong PhysicsDefs.h
    // Cho WORLD va chạm với mọi “thứ rắn” và nhận contact với player/sensor.
    phys::setMasks(body,
        phys::CAT_WORLD,
        (phys::CAT_PLAYER | phys::CAT_ENEMY | phys::CAT_SOLID | phys::CAT_GATE | phys::CAT_CRATE),
        (phys::CAT_PLAYER | phys::CAT_ENEMY | phys::CAT_SENSOR)
    );

    n->setPhysicsBody(body);

    auto d = DrawNode::create();
    d->drawSolidRect(Vec2(-w*0.5f, -h*0.5f), Vec2(w*0.5f, h*0.5f), c);
    n->addChild(d);

    root->addChild(n);
    return n;
}

static void addZoneBG(Node* root, const Size& vs, const Vec2& origin, int i) {
    auto bg = LayerColor::create(Color4B(25+10*i, 25+10*i, 30+6*i, 255));
    bg->setContentSize(vs);
    bg->setPosition(origin + Vec2(i * vs.width, 0));
    root->addChild(bg, -90);
}

} // anon

namespace levels {

BuildResult buildLevel1(Node* root, const Size& vs, const Vec2& origin) {
    BuildResult out{};

    const int   SEGMENTS    = 5;
    const float SEG_W       = vs.width;
    const float GROUND_H    = 36.f;
    const float GROUND_TOP  = origin.y + vs.height * 0.12f;

    out.segments     = SEGMENTS;
    out.segmentWidth = SEG_W;
    out.groundTop    = GROUND_TOP;

    // Ground chạy suốt map (top-anchored)
    const float worldW = SEG_W * SEGMENTS;
    addPlatformTop(root, origin.x + worldW*0.5f, GROUND_TOP,
                   worldW, GROUND_H, Color4F(0.10f,0.82f,0.22f,1.f));

    for (int i=0; i<SEGMENTS; ++i) {
        const float x0 = origin.x + i * SEG_W;
        addZoneBG(root, vs, origin, i);

        switch (i) {
        case 0: {
            addPlatformTop(root, x0 + SEG_W*0.42f, GROUND_TOP + 110.f, 220.f, 16.f, Color4F(0.48f,0.48f,0.78f,1));
            for (int k=0;k<3;++k){
                auto c = Coin::create();
                c->setPosition({x0+SEG_W*0.42f-30.f+30.f*k, GROUND_TOP+135.f});
                root->addChild(c, 2);
            }
            break;
        }
        case 1: {
            addPlatformTop(root, x0 + SEG_W*0.30f, GROUND_TOP + 150.f, 180.f, 16.f, Color4F(0.48f,0.48f,0.78f,1));
            addPlatformTop(root, x0 + SEG_W*0.60f, GROUND_TOP + 200.f, 160.f, 16.f, Color4F(0.40f,0.65f,0.80f,1));
            auto g = Goomba::create(); root->addChild(g,2);
            g->enablePhysics({x0+SEG_W*0.50f, GROUND_TOP+10.f}, {42,42});
            g->setPatrol({x0+SEG_W*0.45f, GROUND_TOP+10.f},{x0+SEG_W*0.70f, GROUND_TOP+10.f});
            out.enemies.pushBack(g);
            break;
        }
        case 2: {
            addPlatformTop(root, x0 + SEG_W*0.25f, GROUND_TOP + 140.f, 140.f, 16.f, Color4F(0.40f,0.40f,0.70f,1));
            addPlatformTop(root, x0 + SEG_W*0.50f, GROUND_TOP + 180.f, 200.f, 16.f, Color4F(0.48f,0.48f,0.78f,1));
            addPlatformTop(root, x0 + SEG_W*0.75f, GROUND_TOP + 220.f, 140.f, 16.f, Color4F(0.35f,0.60f,0.75f,1));
            auto s = Spiker::create(); root->addChild(s,2);
            s->enablePhysics({x0+SEG_W*0.58f, GROUND_TOP+10.f}, {42,42});
            s->setPatrol({x0+SEG_W*0.52f, GROUND_TOP+10.f},{x0+SEG_W*0.70f, GROUND_TOP+10.f});
            out.enemies.pushBack(s);
            for (int k=0;k<3;++k){
                auto c = Coin::create();
                c->setPosition({x0+SEG_W*0.50f-30.f+30.f*k, GROUND_TOP+205.f});
                root->addChild(c, 2);
            }
            break;
        }
        case 3: {
            addPlatformTop(root, x0 + SEG_W*0.35f, GROUND_TOP + 160.f, 180.f, 16.f, Color4F(0.48f,0.48f,0.78f,1));
            addPlatformTop(root, x0 + SEG_W*0.65f, GROUND_TOP + 120.f, 200.f, 16.f, Color4F(0.40f,0.65f,0.80f,1));
            break;
        }
        case 4: {
            addPlatformTop(root, x0 + SEG_W*0.50f, GROUND_TOP + 160.f, 220.f, 16.f, Color4F(0.48f,0.48f,0.78f,1));
            auto boss = BossGolem::create(); root->addChild(boss,2);
            boss->enablePhysics({x0+SEG_W*0.70f, GROUND_TOP+10.f}, {72,72});
            boss->setPatrol({x0+SEG_W*0.62f, GROUND_TOP+10.f},{x0+SEG_W*0.88f, GROUND_TOP+10.f});
            out.enemies.pushBack(boss);

            auto star = Star::create();
            star->setPosition({x0+SEG_W*0.52f, GROUND_TOP+190.f});
            root->addChild(star,2);
            break;
        }
        }
    }

    out.playerSpawn = Vec2(origin.x + SEG_W*0.15f, GROUND_TOP + 1.f); // feet = top + 1
    return out;
}


} // namespace levels
