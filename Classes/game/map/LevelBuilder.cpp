// Classes/game/map/LevelBuilder.cpp
#include "game/map/LevelBuilder.h"
#include "physics/PhysicsDefs.h"

#include "game/Enemy.h"
#include "game/enemies/Goomba.h"
#include "game/enemies/Spiker.h"
#include "game/bosses/BossGolem.h"

#include "game/objects/Coin.h"
#include "game/objects/Chest.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Star.h"
#include "2d/CCDrawNode.h"
#include "game/enemies/Goomba.h"
#include "game/enemies/Spiker.h"
#include "2d/CCParallaxNode.h"


USING_NS_CC;
namespace levels {

// ===== tuning =====
static const int   HP_BY_SEG[5]   = {3, 4, 5, 7, 36}; // boss ở seg5 dùng trong BossGolem
static const float SPD_BY_SEG[5]  = {70.f, 85.f, 95.f, 110.f, 0.f};
static const Upgrade::Type UPG_BY_SEG[5] = {
    Upgrade::Type::SPEED,
    Upgrade::Type::JUMP,
    Upgrade::Type::DAMAGE,
    Upgrade::Type::BULLET,
    Upgrade::Type::DOUBLEJUMP
};



static Node* makeSolid(Node* root, const Rect& r, const Color4F& col, const std::string& texturePath = "", const Vec2& startOffset = Vec2::ZERO) {
    Node* visualNode = nullptr;

    if (!texturePath.empty()) {
        // Nếu có đường dẫn texture, tạo Sprite và lặp lại nếu cần
        // Giả sử bạn có một sprite có kích thước 32x32 cho mỗi tile
        const float TILE_SIZE = 32.0f; // Kích thước của 1 tile trong PNG
        
        // Tạo một Node chứa nhiều Sprite con để tạo hiệu ứng lặp lại
        visualNode = Node::create();
        visualNode->setPosition(r.origin); // Đặt vị trí gốc của node này tại góc dưới trái của Rect

        // Tính số lượng tile cần thiết theo chiều rộng và cao
        int numTilesX = std::ceil(r.size.width / TILE_SIZE);
        int numTilesY = std::ceil(r.size.height / TILE_SIZE);

        for (int y = 0; y < numTilesY; ++y) {
            for (int x = 0; x < numTilesX; ++x) {
                auto tileSprite = Sprite::create(texturePath);
                if (tileSprite) {
                    tileSprite->setAnchorPoint(Vec2(0, 0)); // Quan trọng: Đặt anchor ở góc dưới trái
                    tileSprite->setPosition(startOffset.x + x * TILE_SIZE, startOffset.y + y * TILE_SIZE); // <-- Áp dụng offset
                    // Có thể cần scale nếu tile trong PNG không đúng 32x32 hoặc muốn điều chỉnh
                    // tileSprite->setScale(TILE_SIZE / tileSprite->getContentSize().width);
                    visualNode->addChild(tileSprite);
                }
            }
        }
        root->addChild(visualNode, 1); // Z-order cho visual node

    } else {
        // Nếu không có texture, quay lại vẽ màu đặc như cũ
        auto dn = DrawNode::create();
        dn->drawSolidRect(r.origin, r.origin + r.size, col);
        root->addChild(dn, 1);
        visualNode = dn; // Sử dụng DrawNode làm visual node
    }
    
    // Phần Physics Body vẫn giữ nguyên
    auto n = Node::create();
    n->setPosition(Vec2::ZERO); // Physics body không cần di chuyển, nó sẽ dùng offset
    auto body = PhysicsBody::createBox(r.size, PhysicsMaterial(0.1f,0,0.6f), r.origin + r.size*0.5f);
    body->setDynamic(false);
    body->setCategoryBitmask((int)phys::CAT_WORLD);
    body->setCollisionBitmask((int)(phys::CAT_ALL & ~phys::CAT_BULLET));
    body->setContactTestBitmask((int)phys::CAT_ALL);
    n->addComponent(body);
    root->addChild(n, 2);

    return n;
}

static void coinLine(Node* root, const Vec2& start, int n, float stepX){
    for (int i=0;i<n;i++){
        auto c = Coin::create();
        c->setPosition(start + Vec2(i*stepX,0));
        root->addChild(c,5);
    }
}
static void placeChest(Node* root, const Vec2& p){
    if (auto ch = Chest::create()){ ch->setPosition(p); root->addChild(ch,5); }
}
// CHỈ tạo đúng loại, KHÔNG set duration ở đây
static void placeUpgrade(Node* root, Upgrade::Type t, const Vec2& p){
    Upgrade* u = Upgrade::create(t);
    if (!u) u = Upgrade::createRandom();
    if (u) { u->setPosition(p); root->addChild(u, 5); }
}

// Spawns
static Enemy* spawnGoomba(Node* root, const Vec2& pos, const Vec2& pa, const Vec2& pb, int hp, float spd){
    auto e = Goomba::create();
    e->enablePhysics(pos, Size(38,38));
    e->setPatrol(pa, pb);
    e->setMaxHp(hp);
    e->setMoveSpeed(spd);
    root->addChild(e, 4);
    return e;
}
static Enemy* spawnSpiker(Node* root, const Vec2& pos, const Vec2& pa, const Vec2& pb, int hp, float spd){
    auto e = Spiker::create();
    e->enablePhysics(pos, Size(40,40));
    e->setPatrol(pa, pb);
    e->setMaxHp(hp);
    e->setMoveSpeed(spd);
    root->addChild(e, 4);
    return e;
}


BuildResult buildLevel1(Node* root, const Size& vs, const Vec2& origin) {
    cocos2d::Size _vs{};
    cocos2d::Vec2 _origin{};
    _vs     = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();
    cocos2d::ParallaxNode* _parallaxNode = nullptr; 
    const float SCREEN_CENTER_X = origin.x + vs.width * 0.5f; 
    const float SCREEN_CENTER_Y = origin.y + vs.height * 0.5f;
    BuildResult L;
    L.segments     = 5;
    L.segmentWidth = vs.width;
    L.groundTop    = origin.y + 28.f;
    L.playerSpawn  = origin + Vec2(80.f, L.groundTop + 40.f);

    // ground dài
    // makeSolid(root, Rect(origin.x, origin.y, vs.width*L.segments, 24.f), Color4F(0.2f,0.85f,0.2f,1));
    const Vec2 GROUND_TILE_OFFSET(0.0f, -65.0f); // Dịch chuyển tất cả tile ground xuống 75px
    makeSolid(root, Rect(origin.x, origin.y, vs.width*L.segments, 24.f),
              Color4F(0.20f,0.85f,0.20f,1), "sprites/tiles/ground_tile.png", GROUND_TILE_OFFSET);
    
    const float JUMP_DY = 70.f;
    const float GAP_X   = 130.f;
    const float P_W     = 180.f;
    const float P_H     = 16.f;

    for (int s=0; s<L.segments; ++s) {
            _parallaxNode = ParallaxNode::create();
    // Đặt Z-order rất thấp để nó nằm sau tất cả map và đối tượng.
    // addChild(_parallaxNode, -100); 
    root->addChild(_parallaxNode, -100);

    // LỚP BACKGROUND 1 (bgFar): DUY NHẤT VÀ PHỦ ĐẦY MÀN HÌNH
    auto bgFar = Sprite::create("sprites/backgrounds/background.png"); 
    if (bgFar) {
        float bgW = bgFar->getContentSize().width;
        float bgH = bgFar->getContentSize().height;
        
        float scaleX = _vs.width / bgW;
        float scaleY = _vs.height / bgH;
        float scale = std::max(scaleX, scaleY); // Phóng to để che phủ cả width và height
        
        // Căn tâm Sprite để đặt ở giữa màn hình
        bgFar->setAnchorPoint(Vec2(0.5f, 0.5f)); 
        bgFar->setScale(scale); 
        
        // Vị trí: Đặt ở trung tâm Viewport
        float startX = SCREEN_CENTER_X; 
        float startY = SCREEN_CENTER_Y; 
        
        // Tỷ lệ di chuyển: (0.1f, 1.0f)
        _parallaxNode->addChild(bgFar, -1, Vec2(0.1f, 1.0f), Vec2(startX, startY));
    }
    
    // LỚP BACKGROUND 2 (bgNear): CHỈ 1 ẢNH (Không lặp lại)
    auto bgNear = Sprite::create("sprites/backgrounds/mountains.png");
    if (bgNear) {
        // Đặt anchor point ở góc trái dưới (0, 0)
        bgNear->setAnchorPoint(Vec2(0.f, 0.f)); 
        
        // Vị trí X: Bắt đầu từ mép trái màn hình
        float startX = _origin.x; 
        // Vị trí Y: Neo theo _groundTop
        float startY = L.groundTop - 100.f; 

        // Tỷ lệ di chuyển: (0.5f, 1.0f)
        _parallaxNode->addChild(bgNear, 0, Vec2(0.5f, 1.0f), Vec2(startX, startY)); 
    }

        float baseX = origin.x + s*vs.width;
        float g     = L.groundTop;

        // auto platform = [&](float xPix, float y, float w){
        //     return makeSolid(root, Rect(xPix, g+y, w, P_H),
        //                      Color4F(0.55f,0.58f,0.95f,1));
        // };
                const Vec2 PLATFORM_TILE_OFFSET(-15.0f, -22.0f); // Dịch chuyển tất cả tile platform sang phải 10px và xuống 22px


        auto platform = [&](float xPix, float y, float w){
            return makeSolid(root, Rect(xPix, g+y, w, P_H),
                             Color4F(0.55f,0.58f,0.95f,1),"sprites/tiles/platform_tile.png", PLATFORM_TILE_OFFSET);
        };
        
        // // !platform giữa
        // float platformY = L.groundTop + 70.f;
        // makeSolid(root, Rect(baseX + vs.width*0.40f, platformY, 220.f, 16.f), Color4F(0.55f,0.58f,0.95f,1), "sprites/tiles/platform_tile.png", PLATFORM_TILE_OFFSET);

        int   EHP = HP_BY_SEG[s];
        float ESP = SPD_BY_SEG[s];
        auto  UT  = UPG_BY_SEG[s];

        switch (s) {
        // ===== Segment 1 — tutorial, Goomba chậm, nhảy bậc thang
        case 0: {
            float x1 = baseX + vs.width*0.22f, y1 =  80.f;
            float x2 = std::min(x1 + GAP_X, baseX + vs.width*0.42f), y2 = y1 + JUMP_DY;
            float x3 = std::min(x2 + GAP_X, baseX + vs.width*0.62f), y3 = y2 + JUMP_DY;
            platform(x1, y1, P_W);
            platform(x2, y2, P_W);
            platform(x3, y3, P_W-10.f);

            // trụ cản nho nhỏ cho đỡ trống
            const Vec2 WALL_TILE_OFFSET(0.0f, -85.0f);
            makeSolid(root, Rect(baseX+vs.width*0.78f, g, 20, 70), Color4F(0.25f,0.8f,0.25f,1),"sprites/tiles/wall_tile.png", WALL_TILE_OFFSET);

            coinLine(root, {baseX + 140, g+120}, 4, 36.f);
            placeUpgrade(root, UT, {x2 + P_W*0.5f, g + y2 + 26.f});
            placeChest(root, {baseX + vs.width*0.84f, g+36});

            auto e = spawnGoomba(root, {x2 + P_W*0.5f, g+y2 + 22.f},
                                 {x2-80.f, g+y2 + 22.f}, {x2+80.f, g+y2 + 22.f},
                                 EHP, ESP);
            L.enemies.pushBack(e);

            auto star = Star::create();
            star->setPosition({x3 + P_W*0.5f, g+y3 + 34.f});
            root->addChild(star, 6);
        } break;

        // ===== Segment 2 — Spiker, nhảy chéo
        case 1: {
            float x1 = baseX + vs.width*0.24f, y1 =  90.f;
            float x2 = std::min(x1 + GAP_X, baseX + vs.width*0.54f), y2 = y1 + JUMP_DY;
            platform(x1, y1, P_W-10.f);
            platform(x2, y2, P_W);

            // cột đôi
            makeSolid(root, Rect(baseX+vs.width*0.68f, g, 18, 76), Color4F(0.25f,0.8f,0.25f,1));
            makeSolid(root, Rect(baseX+vs.width*0.73f, g, 18, 60), Color4F(0.25f,0.8f,0.25f,1));

            coinLine(root, {baseX + 160, g+180}, 4, 36.f);
            placeUpgrade(root, UT, {x1 + P_W*0.5f, g+y1 + 26.f});
            placeChest(root, {baseX + vs.width*0.82f, g+36});

            auto e = spawnSpiker(root, {x2 + P_W*0.5f, g+y2 + 22.f},
                                 {x2-90.f, g+y2 + 22.f}, {x2+90.f, g+y2 + 22.f},
                                 EHP, ESP);
            L.enemies.pushBack(e);

            auto star = Star::create();
            star->setPosition({x2 + P_W*0.3f, g+y2 + 60.f});
            root->addChild(star, 6);
        } break;

        // ===== Segment 3 — Mix: goomba + spiker, platform rộng
        case 2: {
            float x1 = baseX + vs.width*0.18f, y1 =  90.f;
            float x2 = std::min(x1 + GAP_X, baseX + vs.width*0.48f), y2 = y1 + JUMP_DY;
            platform(x1, y1, P_W-20.f);
            platform(x2, y2, P_W+30.f);

            coinLine(root, {baseX + 160, g+165}, 5, 34.f);
            placeUpgrade(root, UT, {x1 + P_W*0.3f, g+y1 + 24.f});
            placeChest(root, {baseX + vs.width*0.80f, g+36});

            auto e1 = spawnGoomba(root, {x1 + P_W*0.3f, g+y1 + 20.f},
                                  {x1-60.f, g+y1 + 20.f}, {x1+60.f, g+y1 + 20.f},
                                  EHP, ESP-10.f);
            auto e2 = spawnSpiker(root, {x2 + P_W*0.4f, g+y2 + 22.f},
                                  {x2-90.f, g+y2 + 22.f}, {x2+90.f, g+y2 + 22.f},
                                  EHP+1, ESP);
            L.enemies.pushBack(e1); L.enemies.pushBack(e2);

            auto star = Star::create();
            star->setPosition({x2 + P_W*0.55f, g+y2 + 60.f});
            root->addChild(star, 6);
        } break;

        // ===== Segment 4 — Spiker nhanh, trụ cao dạng “cánh cổng”
        case 3: {
            float x1 = baseX + vs.width*0.30f, y1 = 120.f;
            float x2 = std::min(x1 + GAP_X, baseX + vs.width*0.64f), y2 = y1 + JUMP_DY;
            platform(x1, y1, P_W);
            platform(x2, y2, P_W-20.f);

            // cổng
            makeSolid(root, Rect(baseX+vs.width*0.20f, g, 22, 96), Color4F(0.25f,0.8f,0.25f,1));
            makeSolid(root, Rect(baseX+vs.width*0.86f, g, 22, 96), Color4F(0.25f,0.8f,0.25f,1));

            coinLine(root, {baseX + 120, g+210}, 6, 30.f);
            placeUpgrade(root, UT, {x1 + P_W*0.4f, g+y1 + 26.f});
            placeChest(root, {baseX + vs.width*0.84f, g+36});

            auto e = spawnSpiker(root, {x1 + P_W*0.4f, g+y1 + 22.f},
                                 {x1-90.f, g+y1 + 22.f}, {x1+90.f, g+y1 + 22.f},
                                 EHP+2, ESP+8.f);
            L.enemies.pushBack(e);

            auto star = Star::create();
            star->setPosition({x2 + P_W*0.4f, g+y2 + 65.f});
            root->addChild(star, 6);
        } break;

        // ===== Segment 5 — BOSS (không đặt Star tĩnh; Star rơi từ boss)
        default: {
            float x1 = baseX + vs.width*0.22f, y1 = 110.f;
            float x2 = std::min(x1 + GAP_X, baseX + vs.width*0.48f), y2 = y1 + JUMP_DY;
            float x3 = std::min(x2 + GAP_X, baseX + vs.width*0.74f), y3 = y2 + JUMP_DY;
            platform(x1, y1, P_W-10.f);
            platform(x2, y2, P_W+10.f);
            platform(x3, y3, P_W-20.f);

            coinLine(root, {baseX + 140, g+150}, 6, 34.f);
            placeUpgrade(root, UT, {x1 + P_W*0.4f, g+y1 + 26.f});
            placeChest(root, {baseX + vs.width*0.60f, g+36});

            auto boss = BossGolem::create();
            boss->enablePhysics({baseX + vs.width*0.68f, g + 40.f}, Size(72,72));
            // Mặc định boss “ngủ”, GameScene sẽ bật aggro ở khúc cuối
            boss->setAggroEnabled(false);
            root->addChild(boss, 4);
            L.enemies.pushBack(boss);
            // KHÔNG tạo Star tĩnh ở segment boss
        } break;
        }
    }
    return L;
}

} // namespace levels
