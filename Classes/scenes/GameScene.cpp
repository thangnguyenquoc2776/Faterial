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
        if (dynamic_cast<Star*>(item))      { _setStars(_starsHave+1, _starsNeed); _addScore(50); }
        else if (dynamic_cast<Coin*>(item)) { _addScore(10); }
        else if (dynamic_cast<Upgrade*>(item)) { _addScore(25); } // chừa chỗ cắm UpgradeSystem sau
        item->removeFromParent();
        _checkWin();
        return false;
    }

    // Bullet/Slash ↔ Enemy
    auto isSlash = [&](PhysicsShape* s)->bool { return s && s->getTag()==(int)phys::ShapeTag::SLASH; };
    Node* enemy=nullptr;
    if ( (hasCat(a,phys::CAT_BULLET)||isSlash(A)) && hasCat(b,phys::CAT_ENEMY)) enemy=b;
    else if ( (hasCat(b,phys::CAT_BULLET)||isSlash(B)) && hasCat(a,phys::CAT_ENEMY)) enemy=a;
    if (enemy) {
        if (auto e = dynamic_cast<Enemy*>(enemy)) e->takeHit(isSlash(A)||isSlash(B)?2:1);
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

void GameScene::update(float) {
    if (_gameOver || _gameWin || !_player) return;

    float x = _player->getPositionX();
    float target = cocos2d::clampf(x, _camL + _vs.width*0.5f, _camR - _vs.width*0.5f);
    this->getScene()->getDefaultCamera()->setPositionX(target);

    if (_player->getPositionX() > _camR - 4.0f && _segment < _segmentCount-1) {
        _segment++;
        _camL = _origin.x + _segment * _segmentWidth;
        _camR = _camL + _segmentWidth;
        if (_hud) _hud->setZone(_segment+1, _segmentCount);
    }

    // Rơi khỏi map → trừ mạng + hồi full HP
    if (_player->getPositionY() < _groundTop - 200.f) {
        _setLives(_lives-1);
        if (_lives<=0) {
            _gameOver=true; _showOverlay("YOU DIED\nPress [R] to restart");
        } else {
            _player->restoreFullHP();
            if (_hud) _hud->setHP(_player->hp(), _player->maxHp());
            _player->setPosition(_origin + Vec2(_segment * _segmentWidth + _vs.width*0.15f,
                                                _groundTop + 40.f));
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
