#include "game/map/LevelBuilder.h"

#include "physics/PhysicsDefs.h"
#include "2d/CCDrawNode.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"

// objects / enemies
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/enemies/Goomba.h"
#include "game/enemies/Spiker.h"
#include "game/bosses/BossGolem.h"

using namespace cocos2d;

namespace levels {

// helper: vẽ 1 hộp tĩnh để debug
static void drawSolidBox(Node* n, const Size& sz, const Color4F& c) {
    auto d = DrawNode::create();
    d->drawSolidRect(Vec2(-sz.width*0.5f, -sz.height*0.5f),
                     Vec2( sz.width*0.5f,  sz.height*0.5f), c);
    n->addChild(d);
}

// helper: tạo ground/world. Position tại (xCenter, yTop - h/2) để đỉnh == yTop
static Node* makeStaticBoxTop(Node* parent, Vec2 centerXTop, Size sz,
                              const Color4F& color, bool world = true)
{
    auto n = Node::create();
    auto body = PhysicsBody::createBox(sz, PhysicsMaterial(0.2f, 0.f, 1.f));
    body->setDynamic(false);
    if (world) {
        body->setCategoryBitmask(static_cast<int>(phys::CAT_WORLD));
        body->setCollisionBitmask(0xFFFFFFFFu);
        body->setContactTestBitmask(0xFFFFFFFFu);
    }
    n->setPhysicsBody(body);
    n->setPosition(centerXTop.x, centerXTop.y - sz.height * 0.5f);
    drawSolidBox(n, sz, color);
    parent->addChild(n);
    return n;
}

// helper: platform với toạ độ đầu vào là Y đỉnh (yTop)
static Node* platformTop(Node* parent, float xCenter, float yTop,
                         float w, float h, const Color4F& c)
{
    return makeStaticBoxTop(parent, {xCenter, yTop}, {w, h}, c, true);
}

LevelBuildResult buildLevel1(Node* parent, const Size& vs, const Vec2& origin)
{
    LevelBuildResult R;
    R.segments     = 5;
    R.segmentWidth = vs.width;

    // --- Ground (đỉnh mặt đất) ---
    const float groundH   = 28.f;
    R.groundTop           = origin.y + 64.f;             // đỉnh nền (nơi đứng)
    const float worldW    = vs.width * R.segments;
    auto ground = makeStaticBoxTop(parent,
                                   {origin.x + worldW * 0.5f, R.groundTop},
                                   {worldW, groundH},
                                   Color4F(0.12f, 0.85f, 0.25f, 1.f), true);
    (void)ground;

    // --- Player spawn (đứng trên mặt đất một chút) ---
    R.playerSpawn = origin + Vec2(vs.width * 0.15f, R.groundTop + 40.f);

    // --- Mỗi đoạn: nền màu + bậc leo + coin/star + enemy patrol ---
    for (int i = 0; i < R.segments; ++i) {
        float x0 = origin.x + i * R.segmentWidth;

        // BG nhẹ cho mỗi đoạn
        auto bg = LayerColor::create(Color4B(30+10*i, 34+5*i, 45+2*i, 255));
        bg->setContentSize(vs);
        bg->setPosition(origin + Vec2(i * R.segmentWidth, 0));
        parent->addChild(bg, -90);

        // Bậc leo — đảm bảo có đường đi lên tự nhiên
        // các yTop là ĐỈNH (mặt trên), nên player đứng “trên” đúng ý bạn
        platformTop(parent, x0 + 240.f, R.groundTop + 120.f, 180.f, 16.f, Color4F(0.45f,0.45f,0.8f,1));
        platformTop(parent, x0 + 480.f, R.groundTop + 170.f, 180.f, 16.f, Color4F(0.45f,0.45f,0.8f,1));
        platformTop(parent, x0 + 660.f, R.groundTop + 140.f, 150.f, 16.f, Color4F(0.35f,0.65f,0.8f,1));

        // Một “chướng ngại” thấp để tập nhảy
        platformTop(parent, x0 + 420.f, R.groundTop + 46.f, 36.f, 36.f, Color4F(0.75f,0.3f,0.3f,1));

        // Coins
        for (int k=0; k<3; ++k) {
            auto c = Coin::create();
            c->setPosition({x0 + 400.f + k*40.f, R.groundTop + 190.f});
            parent->addChild(c, 2);
        }
        // Mỗi đoạn (trừ cuối) có 1 Star để đếm tiến độ
        if (i < R.segments-1) {
            auto s = Star::create();
            s->setPosition({x0 + 520.f, R.groundTop + 210.f});
            parent->addChild(s, 2);
        }

        // Enemies theo độ khó tăng dần
        Enemy* e = nullptr;
        if (i < 2) {
            e = Goomba::create(); parent->addChild(e, 2);
            e->enablePhysics({x0+420.f, R.groundTop+60.f}, {42.f,42.f});
            e->setPatrol({x0+360.f, R.groundTop+60.f}, {x0+520.f, R.groundTop+60.f});
        } else if (i < 4) {
            e = Spiker::create(); parent->addChild(e, 2);
            e->enablePhysics({x0+560.f, R.groundTop+60.f}, {42.f,42.f});
            e->setPatrol({x0+520.f, R.groundTop+60.f}, {x0+640.f, R.groundTop+60.f});
        } else { // boss ở đoạn cuối
            e = BossGolem::create(); parent->addChild(e, 2);
            e->enablePhysics({x0+720.f, R.groundTop+80.f}, {72.f,72.f});
            e->setPatrol({x0+680.f, R.groundTop+80.f}, {x0+820.f, R.groundTop+80.f});
        }
        if (e) R.enemies.pushBack(e);
    }

    return R;
}

} // namespace levels
