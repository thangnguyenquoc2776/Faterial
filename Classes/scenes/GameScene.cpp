#include "scenes/GameScene.h"
#include "ui/HUDLayer.h"
#include "game/map/LevelBuilder.h"

#include "game/weapon/Bullet.h"
#include "game/weapon/Slash.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/objects/Upgrade.h"

#include "physics/PhysicsDefs.h"
#include <algorithm>

USING_NS_CC;

// ================== Bitmasks nội bộ (an toàn) ==================
// Nếu bạn đã có phys::CAT_* trong PhysicsDefs.h thì phần dưới chỉ dùng để "đọc"
// Còn va chạm chính vẫn dựa trên Tag/loại Node cụ thể (Bullet, Slash, Item...).
static constexpr uint32_t PHYS_CAT_WORLD  = 1u << 0;
static constexpr uint32_t PHYS_CAT_PLAYER = 1u << 1;
static constexpr uint32_t PHYS_CAT_ENEMY  = 1u << 2;
static constexpr uint32_t PHYS_CAT_ITEM   = 1u << 3;
static constexpr uint32_t PHYS_CAT_BULLET = 1u << 4;

static bool hasCat(Node* n, uint32_t catMask){
    return n && n->getPhysicsBody() &&
           ((uint32_t)n->getPhysicsBody()->getCategoryBitmask() & catMask) != 0u;
}
static bool isGroundCandidate(Node* n){
    if (!n || !n->getPhysicsBody()) return false;
    auto* b = n->getPhysicsBody();
    // Đất/platform thường là static hoặc có CAT_WORLD
    if (!b->isDynamic()) return true;
#ifdef PHYSICS_DEFS_GUARD
    // Nếu dự án có phys::CAT_WORLD|CAT_SOLID|CAT_GATE thì dùng thêm:
    if ( (b->getCategoryBitmask() & (phys::CAT_WORLD | phys::CAT_SOLID | phys::CAT_GATE)) != 0u ) return true;
#endif
    return (b->getCategoryBitmask() & PHYS_CAT_WORLD) != 0u;
}

// -------------------------------------------------------------------------
// Scene tạo bằng createWithPhysics() — KHÔNG đụng _world trong init()
Scene* GameScene::createScene() {
    auto scene = Scene::createWithPhysics();
    auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}

bool GameScene::init() {
    if (!Layer::init()) return false;

    // KHÔNG lấy _world ở đây
    _vs     = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();

    // UI
    buildUICamera();
    buildHUD();

    // ===== Build Level 1 (map 5 đoạn liên tiếp) =====
    auto L = levels::buildLevel1(this, _vs, _origin);
    _groundTop    = L.groundTop;
    _segmentCount = L.segments;
    _segmentWidth = L.segmentWidth;

    _segment = 0;
    _camL = _origin.x + _segment * _segmentWidth;
    _camR = _camL + _segmentWidth;

    // Player CENTER-anchored
    _player = Player::create();
    addChild(_player, 5);
    const float H  = _player->colliderSize().height;
    // CENTER.y = groundTop + skin + H/2  (tránh kẹt mép)
    const float skin = 1.0f;
    const float cy   = _groundTop + skin + H * 0.5f;
    _player->enablePhysics({ L.playerSpawn.x, cy });

    // Enemies đã add vào root trong builder; gom tham chiếu (nếu cần)
    for (auto* e : L.enemies) _enemies.pushBack(e);

    // HUD init
    _lives = 3; _score = 0; _starsHave = 0; _starsNeed = 5;
    if (_hud) {
        _hud->setZone(1, _segmentCount);
        _hud->setLives(_lives);
        _hud->setScore(_score);
        _hud->setStars(_starsHave, _starsNeed);
    }

    // Input & Contact
    _bindInput();

    auto cl = EventListenerPhysicsContact::create();
    cl->onContactBegin    = CC_CALLBACK_1(GameScene::_onContactBegin, this);
    cl->onContactSeparate = CC_CALLBACK_1(GameScene::_onContactSeparate, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(cl, this);
    _contact = cl;

    scheduleUpdate();
    return true;
}

void GameScene::onEnter() {
    Layer::onEnter();
    // LÚC NÀY layer ĐÃ ở trong Scene → an toàn lấy world
    _scene = this->getScene();
    CCASSERT(_scene, "GameScene must be attached to a Scene");
    _world = _scene->getPhysicsWorld();
    CCASSERT(_world, "Use Scene::createWithPhysics() for GameScene");

    _world->setGravity(Vec2(0, -980)); // ~ -9.8 m/s^2 với 100px ~ 1m
    // Bật khi cần soi:
    // _world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
}

void GameScene::onExit() {
    // Hủy listener nếu cần (engine tự dọn cho child, nhưng rõ ràng hơn)
    if (_kb)      _eventDispatcher->removeEventListener(_kb), _kb=nullptr;
    if (_contact) _eventDispatcher->removeEventListener(_contact), _contact=nullptr;
    Layer::onExit();
}

// ----------------- UI/HUD -----------------
void GameScene::buildUICamera() {
    _uiCam = Camera::createOrthographic(_vs.width, _vs.height, 1.0f, 1024.0f);
    _uiCam->setCameraFlag(CameraFlag::USER1);
    _uiCam->setPosition(_origin + Vec2(_vs.width*0.5f, _vs.height*0.5f));
    addChild(_uiCam, 999);
}
void GameScene::buildHUD() {
    _hud = HUDLayer::create();
    _hud->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(_hud, 100);
}

// ----------------- Input -----------------
void GameScene::_bindInput() {
    auto l = EventListenerKeyboard::create();
    l->onKeyPressed = [this](EventKeyboard::KeyCode c, Event*) {
        // Toggle debug collider
        if (c == EventKeyboard::KeyCode::KEY_F1) {
            if (auto* w = getScene()->getPhysicsWorld()) {
                static bool dbg=false; dbg=!dbg;
                w->setDebugDrawMask(dbg ? PhysicsWorld::DEBUGDRAW_ALL
                                        : PhysicsWorld::DEBUGDRAW_NONE);
            }
            return;
        }

        if (_gameOver || _gameWin) {
            if (c==EventKeyboard::KeyCode::KEY_R && _gameOver) _restartLevel();
            if ((c==EventKeyboard::KeyCode::KEY_ENTER || c==EventKeyboard::KeyCode::KEY_KP_ENTER) && _gameWin) _returnMenu();
            return;
        }
        if (!_player) return;

        switch (c) {
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _player->setMoveDir({-1.f,0.f}); break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _player->setMoveDir({ 1.f,0.f}); break;
            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:
            case EventKeyboard::KeyCode::KEY_SPACE: _player->jump(); break;
            case EventKeyboard::KeyCode::KEY_J: _doShoot(); break;
            case EventKeyboard::KeyCode::KEY_K: _doSlash(); break;
            default: break;
        }
    };
    l->onKeyReleased = [this](EventKeyboard::KeyCode c, Event*) {
        if (_gameOver || _gameWin || !_player) return;
        if (c==EventKeyboard::KeyCode::KEY_A || c==EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            c==EventKeyboard::KeyCode::KEY_D || c==EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
            _player->setMoveDir({0.f,0.f});
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;
}

// ----------------- Combat helpers -----------------
void GameScene::_doShoot(){
    if(!_player) return;
    int dir = _player->facing();
    // Player CENTER: miệng súng đặt cao hơn feet một chút
    Vec2 origin = _player->getPosition() + Vec2(dir*18.f, _player->halfH() * 0.25f);
    if (auto b = Bullet::create(origin, Vec2(700.f * dir, 0.f), 1.5f)) addChild(b, 6);
}
void GameScene::_doSlash(){
    if(!_player) return;
    int dir = _player->facing();
    Vec2 origin = _player->getPosition() + Vec2(dir*28.f, _player->halfH() * 0.1f);
    float angle = (dir > 0) ? 0.0f : 3.14159265f;
    if (auto s = Slash::create(origin, angle, 36.f, 0.12f)) addChild(s, 6);
}

// ----------------- Contact -----------------
bool GameScene::_onContactBegin(PhysicsContact& c) {
    auto A = c.getShapeA(); auto B = c.getShapeB();
    auto a = A->getBody()->getNode(); auto b = B->getBody()->getNode();

    // FOOT ↔ WORLD (điểm chân chạm nền/platform)
    if (A->getTag()==(int)phys::ShapeTag::FOOT && isGroundCandidate(b)) { if (_player) _player->incFoot(1); return true; }
    if (B->getTag()==(int)phys::ShapeTag::FOOT && isGroundCandidate(a)) { if (_player) _player->incFoot(1); return true; }

    // Player ↔ Item (Coin/Star/Upgrade — sensor)
    Node* item = nullptr;
    if (hasCat(a,PHYS_CAT_PLAYER) && hasCat(b,PHYS_CAT_ITEM)) item=b;
    else if (hasCat(b,PHYS_CAT_PLAYER) && hasCat(a,PHYS_CAT_ITEM)) item=a;
    // fallback bằng RTTI nếu mask item chưa chuẩn
    if (!item) {
        if (dynamic_cast<Coin*>(a) || dynamic_cast<Star*>(a) || dynamic_cast<Upgrade*>(a)) item=a;
        else if (dynamic_cast<Coin*>(b) || dynamic_cast<Star*>(b) || dynamic_cast<Upgrade*>(b)) item=b;
    }
    if (item) {
        if (dynamic_cast<Star*>(item))        { _setStars(_starsHave+1, _starsNeed); _addScore(50); }
        else if (dynamic_cast<Coin*>(item))   { _addScore(10); }
        else if (dynamic_cast<Upgrade*>(item)){ _addScore(25); }
        item->removeFromParent();
        _checkWin();
        return false; // không cần xử lý phản lực
    }

    // Bullet/Slash ↔ Enemy
    auto isSlash = [&](PhysicsShape* s)->bool { return s && s->getTag()==(int)phys::ShapeTag::SLASH; };
    Node* enemy=nullptr;
    bool  slashHit = isSlash(A) || isSlash(B);
    if ( (hasCat(a,PHYS_CAT_BULLET)||isSlash(A)) && hasCat(b,PHYS_CAT_ENEMY)) enemy=b;
    else if ( (hasCat(b,PHYS_CAT_BULLET)||isSlash(B)) && hasCat(a,PHYS_CAT_ENEMY)) enemy=a;
    // fallback RTTI nếu mask không đồng bộ
    if (!enemy) {
        if (dynamic_cast<Enemy*>(a) && (hasCat(b,PHYS_CAT_BULLET) || isSlash(B))) enemy=a;
        else if (dynamic_cast<Enemy*>(b) && (hasCat(a,PHYS_CAT_BULLET) || isSlash(A))) enemy=b;
    }
    if (enemy) {
        if (auto e = dynamic_cast<Enemy*>(enemy)) e->takeHit(slashHit ? 2 : 1);
        if (hasCat(a,PHYS_CAT_BULLET)) a->removeFromParent();
        if (hasCat(b,PHYS_CAT_BULLET)) b->removeFromParent();
        _addScore(20);
        return false;
    }

    // Enemy ↔ Player
    if ( (hasCat(a,PHYS_CAT_PLAYER) && hasCat(b,PHYS_CAT_ENEMY)) ||
         (hasCat(b,PHYS_CAT_PLAYER) && hasCat(a,PHYS_CAT_ENEMY)) ) {
        if (_player && !_player->invincible()) {
            Node* eNode = hasCat(a,PHYS_CAT_ENEMY)? a : b;
            float dir = (_player->getPositionX() < eNode->getPositionX()) ? -1.f : 1.f;

            _player->hurt(1);
            if (auto body = _player->getPhysicsBody()) {
                body->applyImpulse(Vec2(-dir*220.f, 260.f));
            }
            _setLives(_lives-1);
            if (_lives<=0) { _gameOver=true; _showOverlay("YOU DIED\nPress [R] to restart"); }
        }
        return true;
    }

    return true;
}

void GameScene::_onContactSeparate(PhysicsContact& c) {
    auto A = c.getShapeA(); auto B = c.getShapeB();
    auto a = A->getBody()->getNode(); auto b = B->getBody()->getNode();
    if (A->getTag()==(int)phys::ShapeTag::FOOT && isGroundCandidate(b)) { if (_player) _player->incFoot(-1); }
    if (B->getTag()==(int)phys::ShapeTag::FOOT && isGroundCandidate(a)) { if (_player) _player->incFoot(-1); }
}

// ----------------- Update -----------------
void GameScene::update(float) {
    if (_gameOver || _gameWin || !_player) return;

    // Camera follow theo X trong biên đoạn hiện tại
    float x = _player->getPositionX();
    float target = cocos2d::clampf(x, _camL + _vs.width*0.5f, _camR - _vs.width*0.5f);
    this->getScene()->getDefaultCamera()->setPositionX(target);

    // Sang đoạn kế (map liên tiếp)
    if (_player->getPositionX() > _camR - 4.0f && _segment < _segmentCount-1) {
        _segment++;
        _camL = _origin.x + _segment * _segmentWidth;
        _camR = _camL + _segmentWidth;
        if (_hud) _hud->setZone(_segment+1, _segmentCount);
    }

    // Rơi khỏi map
    if (_player->getPositionY() < _groundTop - 200.f) {
        _setLives(_lives-1);
        if (_lives<=0) {
            _gameOver=true; _showOverlay("YOU DIED\nPress [R] to restart");
        } else {
            // respawn ở đầu đoạn hiện tại: CENTER.y = groundTop + 40 + H/2
            float cy = _groundTop + 40.f + _player->halfH();
            _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width*0.15f, cy));
        }
    }
}

// ----------------- HUD helpers -----------------
void GameScene::_setLives(int v){
    _lives = std::max(0, v);
    if (_hud) _hud->setLives(_lives);
}
void GameScene::_addScore(int v){
    _score += v;
    if (_hud) _hud->setScore(_score);
}
void GameScene::_setStars(int have, int need){
    _starsHave = have; _starsNeed = need;
    if (_hud) _hud->setStars(_starsHave, _starsNeed);
}
void GameScene::_checkWin(){
    if (_starsHave >= _starsNeed && !_gameWin){
        _gameWin = true;
        _showOverlay("YOU WIN!\nPress [Enter] to Menu");
    }
}

void GameScene::_restartLevel(){
    Director::getInstance()->replaceScene(GameScene::createScene());
}
void GameScene::_returnMenu(){
    Director::getInstance()->replaceScene(TransitionFade::create(0.25f, Scene::create()));
}
void GameScene::_showOverlay(const std::string& text){
    if(!_overlay){
        _overlay = Label::createWithSystemFont(text, "Arial", 46);
        _overlay->setAlignment(TextHAlignment::CENTER);
        _overlay->setAnchorPoint({0.5f,0.5f});
        _overlay->setColor(Color3B::WHITE);
        _overlay->enableShadow();
        addChild(_overlay, 99);
    }
    _overlay->setString(text);
    _overlay->setPosition(_origin + Vec2(_camL + _segmentWidth*0.5f, _vs.height*0.6f));
}
