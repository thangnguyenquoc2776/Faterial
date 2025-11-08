#include "scenes/GameScene.h"
#include "ui/HUDLayer.h"
#include "game/map/LevelBuilder.h"

#include "game/Player.h"                 // cần đầy đủ khai báo Player
#include "game/Enemy.h"
#include "game/bosses/BossGolem.h"
#include "game/weapon/Bullet.h"
#include "game/weapon/Slash.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Chest.h"

#include "physics/PhysicsDefs.h"
#include "physics/CCPhysicsShape.h"
#include "2d/CCDrawNode.h"

#include "ui/PauseLayer.h"
#include "scenes/MenuScene.h"

// Âm thanh (thiếu file -> AudioEngine tự bỏ qua)
#include "audio/Sound.h"

USING_NS_CC;

// ======================================================
// Helpers (free functions)
// ======================================================
static inline bool isSlashShape(cocos2d::PhysicsShape* s) {
    return s && s->getTag() == (int)phys::ShapeTag::SLASH;
}

static inline bool hasCat(cocos2d::Node* n, phys::Mask catMask) {
    if (!n) return false;
    auto* body = n->getPhysicsBody();
    return body && ((static_cast<phys::Mask>(body->getCategoryBitmask()) & catMask) != 0u);
}

//!PICKUP SFX
static inline void playPickupSfx() { snd::sfxCoin(); }

// Overlay chữ giữa màn hình theo vị trí camera mặc định
static void showOverlayNode(cocos2d::Node* root, const std::string& text) {
    if (!root) return;

    float camX = 0.f;
    if (auto sc = root->getScene()) {
        if (auto cam = sc->getDefaultCamera()) camX = cam->getPositionX();
    }
    const auto vs  = Director::getInstance()->getVisibleSize();
    const auto org = Director::getInstance()->getVisibleOrigin();

    Label* label = nullptr;
    if (auto n = root->getChildByName("Overlay")) label = dynamic_cast<Label*>(n);
    if (!label) {
        label = Label::createWithSystemFont(text, "Arial", 46);
        label->setName("Overlay");
        label->setAlignment(TextHAlignment::CENTER);
        label->setAnchorPoint({0.5f, 0.5f});
        label->setColor(Color3B::WHITE);
        label->enableShadow();
        root->addChild(label, 9999);
    }
    label->setString(text);
    label->setPosition(Vec2(camX, org.y + vs.height * 0.6f));
}

// ======================================================

Scene* GameScene::createScene() {
    auto scene = Scene::createWithPhysics();
    auto layer = GameScene::create();
    layer->setPhysicsWorld(scene->getPhysicsWorld());
    scene->addChild(layer);
    return scene;
}

bool GameScene::init() {
    if (!Layer::init()) return false;

    _vs     = Director::getInstance()->getVisibleSize();
    _origin = Director::getInstance()->getVisibleOrigin();

    // ===================== UI Camera =====================
    // Near/Far đối xứng để nhìn thấy z=0; đặt depth > default để vẽ sau cùng
    _uiCam = Camera::createOrthographic(_vs.width, _vs.height, -1024.f, 1024.f);
    _uiCam->setCameraFlag(CameraFlag::USER1);
    _uiCam->setDepth(1);
    _uiCam->setPosition3D(Vec3(_origin.x + _vs.width * 0.5f,
                               _origin.y + _vs.height * 0.5f, 0.f));
    addChild(_uiCam);

    // ===================== HUD =====================
    _hud = HUDLayer::create();
    addChild(_hud, 100);
    // Áp dụng mask ĐỆ QUY: toàn bộ con của HUD đi theo camera USER1
    _hud->setCameraMask((unsigned short)CameraFlag::USER1, true);

    // ===================== Build level =====================
    auto L = levels::buildLevel1(this, _vs, _origin);
    _groundTop    = L.groundTop;
    _segmentCount = L.segments;
    _segmentWidth = L.segmentWidth;
    _worldWidth   = _segmentCount * _segmentWidth;

    // ===================== Player =====================
    _player = Player::create();
    addChild(_player, 5);
    _player->enablePhysics({ L.playerSpawn.x, _groundTop + 220.f });

    // Enemies + Boss cấu hình cơ bản
    for (auto* e : L.enemies) {
        _enemies.pushBack(e);
        e->setTarget(_player);
        e->setAggroEnabled(true);
        e->setAggroRange(220.f);
        e->setChaseSpeed(130.f);
        if (auto boss = dynamic_cast<BossGolem*>(e)) {
            boss->setAggroEnabled(false);   // chỉ bật ở mini cuối
            boss->setAggroRange(320.f);
            boss->setChaseSpeed(110.f);
        }
    }

    // ===================== HUD init =====================
    _segment    = 0;
    _lives      = 3;
    _score      = 0;
    _starsHave  = 0;
    _starsNeed  = _segmentCount;    // 1 sao / mini
    if (_hud) {
        _hud->setZone(1, _segmentCount);
        _hud->setLives(_lives);
        _hud->setScore(_score);
        _hud->setStars(_starsHave, _starsNeed);
        _hud->setHP(_player->hp(), _player->maxHp());
    }

    // ===================== Gates giữa các mini =====================
    {
        const float gateW = 10.f;
        const float gateH = _vs.height * 10.f; // rất cao, không nhảy qua được
        for (int i = 0; i < _segmentCount - 1; ++i) {
            const float x = _origin.x + (i + 1) * _segmentWidth - gateW * 0.5f;
            auto n = Node::create();
            n->setName("Gate_" + std::to_string(i));
            auto body = PhysicsBody::createBox(Size(gateW, gateH));
            body->setDynamic(false);
            body->setRotationEnable(false);
            body->setCategoryBitmask((int)phys::CAT_GATE);
            body->setCollisionBitmask((int)(phys::CAT_PLAYER | phys::CAT_ENEMY));
            body->setContactTestBitmask((int)(phys::CAT_PLAYER | phys::CAT_ENEMY));
            n->addComponent(body);
            n->setPosition(Vec2(x, _groundTop + gateH * 0.5f));
            addChild(n, 2);
        }
    }

    // ===================== Portal cuối (chỉ để nhìn) =====================
    {
        auto dn = DrawNode::create();
        dn->setName("EndPortal");
        dn->drawSolidCircle(Vec2::ZERO, 24, 0, 28, Color4F(0.7f, 0.9f, 1.f, 0.85f));
        dn->setPosition(_origin + Vec2(_worldWidth - 64.f, _groundTop + 64.f));
        addChild(dn, 3);
    }

    // ===================== Input & Contact =====================
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
    if (!_world && getScene()) _world = getScene()->getPhysicsWorld();
    CCASSERT(_world, "GameScene must be under a Scene with physics");
    _world->setGravity(Vec2(0, -980));

    snd::playBgm("audio/bgm_main.mp3", true);
}

void GameScene::onExit() {
    Layer::onExit();
}

// ======================================================================
// Input
// ======================================================================
void GameScene::_bindInput() {
    auto l = EventListenerKeyboard::create();

    l->onKeyPressed = [this](EventKeyboard::KeyCode c, Event*) {
        // Endflow
        if (_gameOver || _gameWin) {
            if (_gameOver && c == EventKeyboard::KeyCode::KEY_R) _restartLevel();
            if (_gameWin &&
                (c == EventKeyboard::KeyCode::KEY_ENTER || c == EventKeyboard::KeyCode::KEY_KP_ENTER))
                _returnMenu();
            return;
        }
        if (!_player) return;

        switch (c) {
            case EventKeyboard::KeyCode::KEY_ESCAPE: {
                if (!this->getChildByName("PauseOverlay")) {
                    auto p = PauseLayer::create();
                    p->setName("PauseOverlay");
                    this->addChild(p, 10000);
                }
            } break;

            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _player->setMoveDir({-1.f, 0.f}); break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _player->setMoveDir({ 1.f, 0.f}); break;

            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:
            case EventKeyboard::KeyCode::KEY_SPACE:       _player->jump();     break;

            case EventKeyboard::KeyCode::KEY_J:           _player->doShoot();  break;
            case EventKeyboard::KeyCode::KEY_K:           _player->doSlash();  break;
            default: break;
        }
    };

    l->onKeyReleased = [this](EventKeyboard::KeyCode c, Event*) {
        if (_gameOver || _gameWin || !_player) return;
        if (c == EventKeyboard::KeyCode::KEY_A ||
            c == EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            c == EventKeyboard::KeyCode::KEY_D ||
            c == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
            _player->setMoveDir({0.f, 0.f});
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
    _kb = l;
}

// ======================================================================
// Contact
// ======================================================================
bool GameScene::_onContactBegin(PhysicsContact& c) {
    auto A = c.getShapeA(); auto B = c.getShapeB();
    auto a = A->getBody()->getNode(); auto b = B->getBody()->getNode();

    // FOOT ↔ WORLD
    if ((A->getTag()==(int)phys::ShapeTag::FOOT && hasCat(b,phys::CAT_WORLD)) ||
        (B->getTag()==(int)phys::ShapeTag::FOOT && hasCat(a,phys::CAT_WORLD))) {
        if (_player) _player->incFoot(1);
        return true;
    }

    // Player ↔ Item
    Node* item = nullptr;
    if (hasCat(a, phys::CAT_PLAYER) && hasCat(b, phys::CAT_ITEM)) item = b;
    else if (hasCat(b, phys::CAT_PLAYER) && hasCat(a, phys::CAT_ITEM)) item = a;

    if (item) {
        int segByItem = (int)((item->getPositionX() - _origin.x) / _segmentWidth);
        segByItem = std::max(0, std::min(segByItem, _segmentCount - 1));

        if (auto star = dynamic_cast<Star*>(item)) {
            snd::sfxStar();
            _setStars(_starsHave + 1, _starsNeed);
            _addScore(50);

            // mở gate mini hiện tại (nếu không phải mini cuối)
            if (segByItem < _segmentCount - 1 && segByItem >= 0) {
                std::string gname = "Gate_" + std::to_string(segByItem);
                if (auto g = this->getChildByName(gname)) {
                    g->removeFromParent();
                    showOverlayNode(this, "Gate opened!");
                }
            }
            item->removeFromParent();
            _checkWin();
            return false;
        }

        if (auto coin = dynamic_cast<Coin*>(item)) {
            playPickupSfx();
            _addScore(10);
            item->removeFromParent();
            return false;
        }

        if (auto up = dynamic_cast<Upgrade*>(item)) {
            using T = Upgrade::Type;
            const auto t = up->type();
            snd::sfxUpgrade();
            _addScore(25);

            if (_player) {
                if (t == T::EXTRA_LIFE) {
                    _setLives(_lives + 1);
                    if (_hud) _hud->addBuff("Extra Life +1", 3.f);
                } else {
                    _player->applyUpgrade((int)t, up->duration());
                    if (_hud) {
                        std::string n;
                        switch (t) {
                            case T::SPEED:      n = "Speed +25%";  break;
                            case T::JUMP:       n = "Jump +15%";   break;
                            case T::DAMAGE:     n = "Damage +1";   break;
                            case T::BULLET:     n = "Bullet +1";   break;
                            case T::RANGE:      n = "Range +";     break;
                            case T::DOUBLEJUMP: n = "Double Jump"; break;
                            default:            n = "Upgrade";     break;
                        }
                        _hud->addBuff(n, std::max(0.1f, up->duration()));
                    }
                }
            }
            item->removeFromParent();
            return false;
        }

        if (auto chest = dynamic_cast<Chest*>(item)) {
            // playPickupSfx();
            snd::sfxUpgrade();
            chest->open();
            return false;
        }
    }

    // Player Bullet/Slash ↔ Enemy Projectile => phá cả hai
    if ( ((hasCat(a, phys::CAT_BULLET) || isSlashShape(A)) && hasCat(b, phys::CAT_ENEMY_PROJ)) ||
         ((hasCat(b, phys::CAT_BULLET) || isSlashShape(B)) && hasCat(a, phys::CAT_ENEMY_PROJ)) ) {

        if (hasCat(a, phys::CAT_ENEMY_PROJ)) a->removeFromParent();
        if (hasCat(b, phys::CAT_ENEMY_PROJ)) b->removeFromParent();
        if (hasCat(a, phys::CAT_BULLET))     a->removeFromParent();
        if (hasCat(b, phys::CAT_BULLET))     b->removeFromParent();
        return false; // chặn phản ứng vật lý mặc định
    }

    // Player ↔ Enemy Projectile => trừ máu + xoá đạn quái
    if ( (hasCat(a, phys::CAT_PLAYER) && hasCat(b, phys::CAT_ENEMY_PROJ)) ||
         (hasCat(b, phys::CAT_PLAYER) && hasCat(a, phys::CAT_ENEMY_PROJ)) ) {

        if (_player && !_player->invincible()) {
            _player->hurt(10);
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());

            if (_player->isDead()) {
                _setLives(_lives - 1);
                if (_lives <= 0) {
                    _gameOver = true;
                    showOverlayNode(this, "YOU DIED\nPress [R] to restart");
                } else {
                    _player->restoreFullHP();
                    if (_hud) _hud->setHP(_player->hp(), _player->maxHp());
                    if (auto body = _player->getPhysicsBody()) body->setVelocity(Vec2::ZERO);
                    _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width * 0.15f,
                                                         _groundTop + 40.f));
                }
            }
        }
        if (hasCat(a, phys::CAT_ENEMY_PROJ)) a->removeFromParent();
        if (hasCat(b, phys::CAT_ENEMY_PROJ)) b->removeFromParent();
        return false;
    }

    // Bullet/Slash ↔ Enemy (thân)
    Node* enemyNode = nullptr;
    if ((hasCat(a, phys::CAT_BULLET) || isSlashShape(A)) && hasCat(b, phys::CAT_ENEMY)) enemyNode = b;
    else if ((hasCat(b, phys::CAT_BULLET) || isSlashShape(B)) && hasCat(a, phys::CAT_ENEMY)) enemyNode = a;

    if (enemyNode) {
        // Có thể va vào node con (hurtbox) -> leo lên cha tìm Enemy
        Node* cur = enemyNode;
        Enemy* e = nullptr;
        while (cur && !(e = dynamic_cast<Enemy*>(cur))) cur = cur->getParent();

        if (e) {
            int base = (isSlashShape(A) || isSlashShape(B)) ? 2 : 1;
            int dmg  = base + (_player ? _player->atkBonus() : 0);
            e->takeHit(dmg);
            _addScore(20);
        }
        if (hasCat(a, phys::CAT_BULLET)) a->removeFromParent();
        if (hasCat(b, phys::CAT_BULLET)) b->removeFromParent();
        return false;
    }

    // Enemy (thân) ↔ Player
    if ((hasCat(a, phys::CAT_PLAYER) && hasCat(b, phys::CAT_ENEMY)) ||
        (hasCat(b, phys::CAT_PLAYER) && hasCat(a, phys::CAT_ENEMY)) ) {
        if (_player && !_player->invincible()) {
            _player->hurt(10);
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());

            // nếu node là projectile gắn tên "enemy_proj" thì dọn luôn
            if (a && a->getName() == "enemy_proj") a->removeFromParent();
            if (b && b->getName() == "enemy_proj") b->removeFromParent();

            if (_player->isDead()) {
                _setLives(_lives - 1);
                if (_lives <= 0) {
                    _gameOver = true; showOverlayNode(this, "YOU DIED\nPress [R] to restart");
                } else {
                    _player->restoreFullHP();
                    if (_hud) _hud->setHP(_player->hp(), _player->maxHp());
                    if (auto body = _player->getPhysicsBody()) body->setVelocity(Vec2::ZERO);
                    _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width * 0.15f,
                                                         _groundTop + 40.f));
                }
            }
        }
        return true;
    }

    return true;
}

void GameScene::_onContactSeparate(PhysicsContact& c) {
    auto A = c.getShapeA(); auto B = c.getShapeB();
    auto a = A->getBody()->getNode(); auto b = B->getBody()->getNode();
    if ((A->getTag()==(int)phys::ShapeTag::FOOT && hasCat(b,phys::CAT_WORLD)) ||
        (B->getTag()==(int)phys::ShapeTag::FOOT && hasCat(a,phys::CAT_WORLD))) {
        if (_player) _player->incFoot(-1);
    }
}

// ======================================================================
// Update
// ======================================================================
void GameScene::update(float dt) {
    if (_gameOver || _gameWin || !_player) return;

    if (_hud) {
        _hud->tick(dt);
        // Đồng bộ HP MỖI KHUNG HÌNH (an toàn tuyệt đối)
        _hud->setHP(_player->hp(), _player->maxHp());
    }

    // Camera follow theo toàn bản đồ
    if (auto* scene = this->getScene()) {
        if (auto* cam = scene->getDefaultCamera()) {
            const float x     = _player->getPositionX();
            const float halfW = _vs.width * 0.5f;
            const float minX  = _origin.x + halfW;
            const float maxX  = _origin.x + std::max(_vs.width, _worldWidth) - halfW;
            cam->setPositionX(cocos2d::clampf(x, minX, maxX));
        }
    }

    // Xác định segment hiện tại
    {
        int idx = (int)((_player->getPositionX() - _origin.x) / _segmentWidth);
        if (idx < 0) idx = 0;
        if (idx > _segmentCount - 1) idx = _segmentCount - 1;
        _segment = idx;
    }
    if (_hud) _hud->setZone(_segment + 1, _segmentCount);

    // Vào mini cuối => bật boss
    if (!_bossAggroOn && _segment == _segmentCount - 1) {
        for (auto* e : _enemies)
            if (auto b = dynamic_cast<BossGolem*>(e)) b->setAggroEnabled(true);
        _bossAggroOn = true;
    }

    // Rơi khỏi map => trừ mạng + respawn
    if (_player->getPositionY() < _groundTop - 200.f) {
        _setLives(_lives - 1);
        if (_lives <= 0) {
            _gameOver = true;
            showOverlayNode(this, "YOU DIED\nPress [R] to restart");
        } else {
            _player->restoreFullHP();
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());
            if (auto body = _player->getPhysicsBody()) body->setVelocity(Vec2::ZERO);
            _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width * 0.15f,
                                                _groundTop + 40.f));
        }
    }

    // Đủ sao → chạm portal để Win
    if (!_gameWin && _starsHave >= _starsNeed && _player) {
        if (auto portal = this->getChildByName("EndPortal")) {
            float dist = _player->getPosition().distance(portal->getPosition());
            if (dist < 36.f) {
                _gameWin = true;
                showOverlayNode(this, "YOU WIN!\nPress [Enter] to Menu");
            }
        }
    }
}

// ======================================================================
// HUD helpers
// ======================================================================
void GameScene::_setLives(int v) { _lives = std::max(0, v); if (_hud) _hud->setLives(_lives); }
void GameScene::_addScore(int v) { _score += v;            if (_hud) _hud->setScore(_score); }
void GameScene::_setStars(int have, int need) {
    _starsHave = have; _starsNeed = need;
    if (_hud) _hud->setStars(_starsHave, _starsNeed);
}
void GameScene::_checkWin() {
    if (_starsHave >= _starsNeed && !_gameWin) {
        _gameWin = true;
        showOverlayNode(this, "YOU WIN!\nPress [Enter] to Menu");
    }
}
void GameScene::_restartLevel() { Director::getInstance()->replaceScene(GameScene::createScene()); }
void GameScene::_returnMenu() {
    Director::getInstance()->replaceScene(TransitionFade::create(0.25f, MenuScene::createScene()));
}
