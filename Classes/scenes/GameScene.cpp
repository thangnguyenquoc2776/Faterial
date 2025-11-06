#include "scenes/GameScene.h"
#include "ui/HUDLayer.h"
#include "game/map/LevelBuilder.h"

#include "game/weapon/Bullet.h"
#include "game/weapon/Slash.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Chest.h"

#include "2d/CCDrawNode.h"

#include "physics/PhysicsDefs.h"
#include <algorithm>

USING_NS_CC;

// helper: check category
static bool hasCat(cocos2d::Node* n, phys::Mask catMask){
    return n && n->getPhysicsBody() &&
           ((phys::Mask)n->getPhysicsBody()->getCategoryBitmask() & catMask) != 0u;
}

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

    buildUICamera();
    buildHUD();

    // Map 5 đoạn
    auto L = levels::buildLevel1(this, _vs, _origin);
    _groundTop    = L.groundTop;
    _segmentCount = L.segments;
    _segmentWidth = L.segmentWidth;

    _segment = 0;
    _camL = _origin.x + _segment * _segmentWidth;
    _camR = _camL + _segmentWidth;

    // Player
    _player = Player::create();
    addChild(_player, 5);
    _player->enablePhysics({ L.playerSpawn.x, _groundTop + 220.f });

    for (auto* e : L.enemies) _enemies.pushBack(e);

    // HUD
    _lives = 3; _score = 0; _starsHave = 0; _starsNeed = _segmentCount;
    if (_hud) {
        _hud->setZone(1, _segmentCount);
        _hud->setLives(_lives);
        _hud->setScore(_score);
        _hud->setStars(_starsHave, _starsNeed);
        _hud->setHP(_player->hp(), _player->maxHp());   // <---
    }


    // --- Sao theo mini + barrier + portal ---
    _starsNeed = _segmentCount;                  // cần 1 sao/mini => tổng 5
    _starsSeg.assign(_segmentCount, 0);

    // Tạo barrier khóa giữa mini i và i+1 (i = 0..3)
    for (int i=0; i<_segmentCount-1; ++i) {
        float x = _origin.x + (i+1)*_segmentWidth - 3.f; // vách ngăn “mỏng”
        auto n = Node::create();
        auto body = PhysicsBody::createBox(Size(6, _vs.height));
        body->setDynamic(false);
        body->setCategoryBitmask((int)phys::CAT_GATE);
        body->setCollisionBitmask((int)phys::CAT_PLAYER);      // chặn Player
        body->setContactTestBitmask((int)phys::CAT_PLAYER);
        n->addComponent(body);
        n->setPosition({x, _groundTop + _vs.height*0.5f});
        addChild(n, 2);
        _locks.push_back(n);
    }

    // Portal cuối (visual thôi, check khoảng cách khi đủ sao)
    auto dn = DrawNode::create();
    dn->drawSolidCircle(Vec2::ZERO, 24, 0, 28, Color4F(0.7f,0.9f,1.f,0.85f));
    _endPortal = dn;
    // đặt ở mép phải mini 5, hơi nổi lên khỏi ground
    _endPortal->setPosition(_origin + Vec2(_segmentCount*_segmentWidth - 64.f, _groundTop + 64.f));
    addChild(_endPortal, 3);


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
}

void GameScene::onExit() { Layer::onExit(); }

// UI
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

// Input
void GameScene::_bindInput() {
    auto l = EventListenerKeyboard::create();
    l->onKeyPressed = [this](EventKeyboard::KeyCode c, Event*) {
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
            case EventKeyboard::KeyCode::KEY_J: _player->doShoot(); break;
            case EventKeyboard::KeyCode::KEY_K: _player->doSlash(); break;
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



// Contact
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
    if (hasCat(a,phys::CAT_PLAYER) && hasCat(b,phys::CAT_ITEM)) item=b;
    else if (hasCat(b,phys::CAT_PLAYER) && hasCat(a,phys::CAT_ITEM)) item=a;

    if (item) {
        // Mini của vật phẩm theo X (0.._segmentCount-1)
        int segByItem = (int)((item->getPositionX() - _origin.x) / _segmentWidth);
        if (segByItem < 0) segByItem = 0;
        if (segByItem >= _segmentCount) segByItem = _segmentCount-1;

        if (auto star = dynamic_cast<Star*>(item)) {
            // Ghi nhận sao
            _starsSeg[segByItem] += 1;
            _setStars(_starsHave+1, _starsNeed);
            _addScore(50);

            // Mở barrier của mini này (nếu chưa phải mini cuối)
            if (segByItem < _segmentCount-1 && segByItem >= 0 && segByItem < (int)_locks.size()) {
                if (_locks[segByItem]) {
                    _locks[segByItem]->removeFromParent();
                    _locks[segByItem] = nullptr;
                    _showOverlay("Gate opened!");
                }
            }

            item->removeFromParent();
            _checkWin();
            return false;
        }

        if (auto coin = dynamic_cast<Coin*>(item)) {
            _addScore(10);
            item->removeFromParent();
            return false;
        }

        if (auto up = dynamic_cast<Upgrade*>(item)) {
            using T = Upgrade::Type;
            const auto t = up->type();

            _addScore(25);
            if (_player) {
                if (t == T::EXTRA_LIFE) {
                    _setLives(_lives + 1); // +1 mạng ngay
                    if (_hud) _hud->addBuff("Extra Life +1", 3.f);
                } else {
                    // Giao kèo: Player có applyUpgrade(int type, float duration)
                    _player->applyUpgrade((int)t, up->duration());

                    if (_hud) {
                        std::string n;
                        switch (t) {
                            case T::SPEED:      n="Speed +25%"; break;
                            case T::JUMP:       n="Jump +15%"; break;
                            case T::DAMAGE:     n="Damage +1"; break;
                            case T::BULLET:     n="Bullet +1"; break;
                            case T::RANGE:      n="Range +"; break;
                            case T::DOUBLEJUMP: n="Double Jump"; break;
                            default:            n="Upgrade"; break;
                        }
                        _hud->addBuff(n, std::max(0.1f, up->duration()));
                    }
                }
            }
            item->removeFromParent();
            return false;
        }

        if (auto chest = dynamic_cast<Chest*>(item)) {
            chest->open();      // để nguyên logic chest
            return false;
        }
    }



    // Bullet/Slash ↔ Enemy
    auto isSlash = [&](PhysicsShape* s)->bool { return s && s->getTag()==(int)phys::ShapeTag::SLASH; };
    Node* enemy=nullptr;
    if ( (hasCat(a,phys::CAT_BULLET)||isSlash(A)) && hasCat(b,phys::CAT_ENEMY)) enemy=b;
    else if ( (hasCat(b,phys::CAT_BULLET)||isSlash(B)) && hasCat(a,phys::CAT_ENEMY)) enemy=a;

    if (enemy) {
        // ➜ GỘP DAMAGE TỪ BUFF PLAYER: base + atkBonus()
        int base = (isSlash(A)||isSlash(B)) ? 2 : 1;
        int dmg  = base + (_player ? _player->atkBonus() : 0);

        if (auto e = dynamic_cast<Enemy*>(enemy)) e->takeHit(dmg);

        if (hasCat(a,phys::CAT_BULLET)) a->removeFromParent();
        if (hasCat(b,phys::CAT_BULLET)) b->removeFromParent();

        _addScore(20);
        return false;
    }



    // Enemy ↔ Player → TRỪ HP, chỉ trừ mạng khi HP hết
    if ( (hasCat(a,phys::CAT_PLAYER) && hasCat(b,phys::CAT_ENEMY)) ||
         (hasCat(b,phys::CAT_PLAYER) && hasCat(a,phys::CAT_ENEMY)) ) {
        if (_player && !_player->invincible()) {
            _player->hurt(10);                                // trừ HP
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());

            if (_player->isDead()) {
                _setLives(_lives-1);
                if (_lives<=0) {
                    _gameOver=true; _showOverlay("YOU DIED\nPress [R] to restart");
                } else {
                    _player->restoreFullHP();
                    if (_hud) _hud->setHP(_player->hp(), _player->maxHp());
                    if (auto body = _player->getPhysicsBody()) body->setVelocity(Vec2::ZERO);
                    _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width*0.15f,
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

void GameScene::update(float dt) {
    if (_gameOver || _gameWin || !_player) return;

    // --- NEW: HUD đếm ngược buff mỗi frame ---
    if (_hud) _hud->tick(dt);

    // --- Camera follow trong biên của đoạn hiện tại ---
    auto* scene = this->getScene();
    auto* cam   = scene ? scene->getDefaultCamera() : nullptr;
    if (cam) {
        float x      = _player->getPositionX();
        float halfW  = _vs.width * 0.5f;
        float target = cocos2d::clampf(x, _camL + halfW, _camR - halfW);
        cam->setPositionX(target);
    }

    // --- Sang đoạn kế (nếu đã tới mép phải đoạn) ---
    if (_player->getPositionX() > _camR - 4.0f && _segment < _segmentCount - 1) {
        _segment++;
        _camL = _origin.x + _segment * _segmentWidth;
        _camR = _camL + _segmentWidth;
        if (_hud) _hud->setZone(_segment + 1, _segmentCount);
    }

    // --- Rơi khỏi map → trừ mạng + hồi full HP + respawn ---
    if (_player->getPositionY() < _groundTop - 200.f) {
        _setLives(_lives - 1);
        if (_lives <= 0) {
            _gameOver = true;
            _showOverlay("YOU DIED\nPress [R] to restart");
        } else {
            _player->restoreFullHP();
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());

            if (auto body = _player->getPhysicsBody())
                body->setVelocity(Vec2::ZERO);   // NEW: reset vận tốc rơi

            // respawn ở đầu đoạn hiện tại
            _player->setPosition(_origin + Vec2(
                _segment * _segmentWidth + _vs.width * 0.15f,
                _groundTop + 40.f
            ));
        }
    }

    // --- NEW: Đủ số sao → chạm gần portal cuối để Win ---
    // Yêu cầu: _endPortal đã được tạo trong init() như mình hướng dẫn
    if (!_gameWin && _starsHave >= _starsNeed && _endPortal && _player) {
        float dist = _player->getPosition().distance(_endPortal->getPosition());
        if (dist < 36.f) {
            _gameWin = true;
            _showOverlay("YOU WIN!\nPress [Enter] to Menu");
        }
    }
}


// HUD helpers
void GameScene::_setLives(int v){ _lives = std::max(0, v); if (_hud) _hud->setLives(_lives); }
void GameScene::_addScore(int v){ _score += v; if (_hud) _hud->setScore(_score); }
void GameScene::_setStars(int have, int need){ _starsHave = have; _starsNeed = need; if (_hud) _hud->setStars(_starsHave, _starsNeed); }
void GameScene::_checkWin(){
    if (_starsHave >= _starsNeed && !_gameWin){
        _gameWin = true;
        _showOverlay("YOU WIN!\nPress [Enter] to Menu");
    }
}
void GameScene::_restartLevel(){ Director::getInstance()->replaceScene(GameScene::createScene()); }
void GameScene::_returnMenu(){ Director::getInstance()->replaceScene(TransitionFade::create(0.25f, Scene::create())); }
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
