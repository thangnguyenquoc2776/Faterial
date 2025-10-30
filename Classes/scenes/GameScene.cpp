#include "scenes/GameScene.h"
#include "ui/HUDLayer.h"
#include "game/Player.h"
#include "game/enemies/Goomba.h"
#include "game/enemies/Spiker.h"
#include "game/bosses/BossGolem.h"
#include "game/objects/Coin.h"
#include "game/objects/Star.h"
#include "game/objects/Upgrade.h"
#include "game/objects/Crate.h"
#include "game/objects/Gate.h"
#include "physics/PhysicsDefs.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;
using AE = cocos2d::AudioEngine;

Scene* GameScene::createScene(){ return GameScene::create(); }

bool GameScene::init(){
    if(!Scene::initWithPhysics()) return false;
    getPhysicsWorld()->setGravity({0, -980});
#if COCOS2D_DEBUG
    getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif
    schedule(CC_SCHEDULE_SELECTOR(GameScene::_followCamera), 0);
    scheduleUpdate(); // kill-zone & HUD tick
    return true;
}

void GameScene::onEnter(){
    Scene::onEnter();
    _setupWorld();
    _hud = HUDLayer::create(); addChild(_hud, 1000);
    _hud->setLives(_lives); _hud->setScore(_score); _hud->setStars(_stars,_starsNeed); _hud->setZone(_zoneIdx,_zoneTot);

    _buildZones();
    _spawnPlayer(Vec2(140.f, 60.f)); // spawn gần đất (đất ở y=12)
    _bindInput();

    auto cl = EventListenerPhysicsContact::create();
    cl->onContactBegin = CC_CALLBACK_1(GameScene::_onContactBegin, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(cl, this);

    _playBgm("Resources/bgm_stage.ogg", true, 0.35f);
}

void GameScene::_setupWorld(){
    auto vs = Director::getInstance()->getVisibleSize();
    _worldW = vs.width * _zoneTot;
    _worldH = vs.height;

    _world = Node::create(); addChild(_world, 0);

    auto bg = LayerColor::create(Color4B(25,25,32,255), _worldW, _worldH);
    _world->addChild(bg, -100);

    // biên ngoài
    auto edge = Node::create();
    auto ebody = PhysicsBody::createEdgeBox(Size(_worldW, _worldH), PhysicsMaterial(0.1f,0,1), 2.0f);
    ebody->setCategoryBitmask(phys::CAT_WORLD);
    ebody->setCollisionBitmask(phys::all());
    ebody->setContactTestBitmask(phys::all());
    edge->setPhysicsBody(ebody);
    edge->setPosition(_worldW/2.f, _worldH/2.f);
    _world->addChild(edge);

    // mặt đất: CAO 24px, nằm SÁT đáy → tâm ở y=12
    auto ground = Node::create();
    float gh = 24.f;
    auto gBody = PhysicsBody::createBox(Size(_worldW*0.98f, gh), PhysicsMaterial(0.2f,0,1));
    gBody->setDynamic(false);
    gBody->setCategoryBitmask(phys::CAT_WORLD);
    gBody->setCollisionBitmask(phys::all());
    gBody->setContactTestBitmask(phys::all());
    ground->setPhysicsBody(gBody);
    ground->setPosition(_worldW*0.5f, gh*0.5f); // y = 12
    auto dn = DrawNode::create();
    dn->drawSolidRect({-(_worldW*0.49f),-gh/2.f},{_worldW*0.49f,gh/2.f}, Color4F(0.15f,0.8f,0.25f,1));
    ground->addChild(dn);
    _world->addChild(ground);
}

void GameScene::_buildZones(){
    auto vs = Director::getInstance()->getVisibleSize();
    const float zoneW = vs.width;
    const float baseY = 12.f;       // mặt đất ở y=12

    auto makePlatform = [&](float x, float y, float w, float h, Color4F col){
        auto n = Node::create(); n->setPosition(x,y);
        auto b = PhysicsBody::createBox(Size(w,h), PhysicsMaterial(0.2f,0,1));
        b->setDynamic(false);
        b->setCategoryBitmask(phys::CAT_WORLD);
        b->setCollisionBitmask(phys::all());
        b->setContactTestBitmask(phys::all());
        n->setPhysicsBody(b);
        auto d = DrawNode::create(); d->drawSolidRect({-w/2.f,-h/2.f},{w/2.f,h/2.f}, col); n->addChild(d);
        _world->addChild(n);
    };

    for(int i=0;i<_zoneTot;++i){
        float cx = zoneW*i + zoneW*0.5f;

        makePlatform(cx-160.f, baseY+110.f, 120.f, 18.f, Color4F(0.35f,0.35f,0.6f,1));
        makePlatform(cx+120.f, baseY+170.f,  90.f, 18.f, Color4F(0.45f,0.35f,0.6f,1));
        makePlatform(cx+ 40.f, baseY+ 60.f,  80.f, 18.f, Color4F(0.35f,0.55f,0.6f,1));

        for(int k=0;k<3;++k){
            auto c = Coin::create();
            c->setPosition(cx-120.f+60.f*k, baseY+140.f);
            _world->addChild(c);
        }

        auto s = Star::create();
        s->setPosition(cx+40.f, baseY+200.f);
        _world->addChild(s);

        auto e = (i%2==0) ? (Enemy*)Goomba::create() : (Enemy*)Spiker::create();
        _world->addChild(e);
        e->enablePhysics(Vec2(cx-200.f, baseY+42.f), Size(42.f,42.f));
        e->setPatrol(Vec2(cx-240.f, baseY+42.f), Vec2(cx-120.f, baseY+42.f));
    }

    // Boss & cổng cuối
    _boss = BossGolem::create(); _world->addChild(_boss);
    float bx = zoneW*(_zoneTot-1) + zoneW*0.70f;
    _boss->enablePhysics(Vec2(bx, baseY+80.f), Size(72.f,72.f));
    _boss->setPatrol(Vec2(bx-80.f, baseY+80.f), Vec2(bx+80.f, baseY+80.f));

    _gateFinal = Gate::create();
    _gateFinal->setPosition(_worldW-60.f, baseY+60.f);
    _world->addChild(_gateFinal);
}

void GameScene::_spawnPlayer(const Vec2& p){
    if(_player) _player->removeFromParent();
    _player = Player::create();
    _world->addChild(_player, 10);
    _player->enablePhysics(p);
    _attachTargetsToEnemies();           // cho AI “nhìn” thấy player
}

void GameScene::_attachTargetsToEnemies(){
    for(auto n : _world->getChildren()){
        if(auto e = dynamic_cast<Enemy*>(n)) e->setTarget(_player);
    }
}

void GameScene::_bindInput(){
    auto l = EventListenerKeyboard::create();
    l->onKeyPressed = [this](EventKeyboard::KeyCode c, Event*){
        if(!_player) return;
        switch(c){
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _player->setMoveDir({-1.f,0.f}); break;
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _player->setMoveDir({ 1.f,0.f}); break;
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
        case EventKeyboard::KeyCode::KEY_SPACE: _player->jump(); break;
        case EventKeyboard::KeyCode::KEY_S:
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            if(auto b=_player->getPhysicsBody())
                b->applyImpulse(Vec2(0.f,-350.f));    // fast-fall
            break;
        case EventKeyboard::KeyCode::KEY_F1:
            getPhysicsWorld()->setDebugDrawMask(
                getPhysicsWorld()->getDebugDrawMask()?0:PhysicsWorld::DEBUGDRAW_ALL); break;
        case EventKeyboard::KeyCode::KEY_R: _spawnPlayer(Vec2(140.f,60.f)); break;
        case EventKeyboard::KeyCode::KEY_ESCAPE: Director::getInstance()->end(); break;
        default: break;
        }
    };
    l->onKeyReleased = [this](EventKeyboard::KeyCode c, Event*){
        if(!_player) return;
        if(c==EventKeyboard::KeyCode::KEY_A || c==EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
           c==EventKeyboard::KeyCode::KEY_D || c==EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
            _player->setMoveDir({0.f,0.f});
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(l, this);
}

bool GameScene::_match(Node* n, uint32_t cat, int tag){
    if(!n || !n->getPhysicsBody()) return false;
    bool catOk = (n->getPhysicsBody()->getCategoryBitmask() & cat) != 0;
    return catOk && (tag==0 ? true : (n->getTag()==tag));
}

bool GameScene::_onContactBegin(PhysicsContact& c){
    auto A = c.getShapeA()->getBody()->getNode();
    auto B = c.getShapeB()->getBody()->getNode();
    if(!A || !B) return true;

    // Player ăn Item
    Node* item=nullptr;
    if(_match(A, phys::CAT_PLAYER) && _match(B, phys::CAT_ITEM)) item=B;
    else if(_match(B, phys::CAT_PLAYER) && _match(A, phys::CAT_ITEM)) item=A;
    if(item){
        switch(static_cast<phys::Tag>(item->getTag())){
            case phys::Tag::COIN:    _addScore(1);  _playSfx("Resources/sfx_pickup_coin.ogg",0.5f); break;
            case phys::Tag::STAR:    _pickupStar(); _playSfx("Resources/sfx_pickup_star.ogg",0.7f); break;
            case phys::Tag::UPGRADE: _lives+=1; if(_hud) _hud->setLives(_lives); _playSfx("Resources/sfx_upgrade.ogg",0.6f); break;
            default: break;
        }
        item->removeFromParent();
        return false;
    }
    return true;
}

void GameScene::update(float){
    if(!_player) return;

    // Kill-zone: rơi khỏi đáy màn hình  ⇒ -1 mạng & respawn
    if(_player->getPositionY() < -40.f){
        _lives = std::max(0, _lives-1);
        if(_hud) _hud->setLives(_lives);
        _spawnPlayer(Vec2(140.f,60.f));
    }

    // Cập nhật Zone theo vị trí camera
    auto vs = Director::getInstance()->getVisibleSize();
    float camX = -_world->getPositionX() + vs.width*0.5f;
    _zoneIdx = std::clamp( (int)std::ceil(camX / vs.width), 1, _zoneTot );
    if(_hud) _hud->setZone(_zoneIdx, _zoneTot);

    _tryOpenFinalGate();
}

void GameScene::_followCamera(float){
    if(!_player || !_world) return;
    auto vs = Director::getInstance()->getVisibleSize();
    float cx = clampf(_player->getPositionX() - vs.width*0.5f, 0.f, _worldW - vs.width);
    _world->setPositionX(-cx);
}

void GameScene::_tryOpenFinalGate(){
    bool bossDead = (_boss==nullptr) || (_boss && !_boss->getParent());
    if(_gateFinal) _gateFinal->open( bossDead && (_stars>=_starsNeed) );
}

void GameScene::_addScore(int s){ _score += s; if(_hud) _hud->setScore(_score); }
void GameScene::_pickupStar(){ _stars += 1; if(_hud) _hud->setStars(_stars,_starsNeed); _tryOpenFinalGate(); }

void GameScene::_playSfx(const std::string& rel, float vol){
    if(FileUtils::getInstance()->isFileExist(rel)) AE::play2d(rel, false, vol);
}
void GameScene::_playBgm(const std::string& rel, bool loop, float vol){
    if(_bgmId>=0){ AE::stop(_bgmId); _bgmId=-1; }
    if(FileUtils::getInstance()->isFileExist(rel)) _bgmId = AE::play2d(rel, loop, vol);
}
