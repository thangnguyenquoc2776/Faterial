#include "game/Player.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;

static const int FOOT_TAG = 1001;

Player* Player::create(){ auto p=new(std::nothrow) Player(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }

bool Player::init(){
    if(!Sprite::init()) return false;
    setTextureRect(Rect(0.f,0.f,40.f,56.f));
    setColor(Color3B::BLUE);

    auto dn=DrawNode::create();
    Vec2 r[4]={{-20.f,-28.f},{20.f,-28.f},{20.f,28.f},{-20.f,28.f}};
    dn->drawPoly(r,4,true,Color4F::WHITE); addChild(dn);

    scheduleUpdate();
    return true;
}

void Player::enablePhysics(const Vec2& pos){
    if(_body) return;
    auto mat = PhysicsMaterial(0.1f, 0.f, 0.f); // friction = 0 để không "kẹt"
    _body = PhysicsBody::createBox(Size(40.f,56.f), mat);
    _body->setDynamic(true);
    _body->setRotationEnable(false);
    _body->setCategoryBitmask(phys::CAT_PLAYER);
    _body->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_ENEMY|phys::CAT_CRATE);
    _body->setContactTestBitmask(phys::all());
    setPhysicsBody(_body);
    setPosition(pos);

    // FOOT SENSOR (8px dày, đặt dưới đáy)
    auto foot = PhysicsShapeBox::create(Size(32.f,8.f), PhysicsMaterial(0,0,0), Vec2(0.f,-28.f));
    foot->setTag(FOOT_TAG);
    foot->setSensor(true);
    _body->addShape(foot);

    _hp=_hpMax; _iframe=0.f;
}

void Player::setMoveDir(const Vec2& dir){
    _moveDir = dir;
    if(dir.x != 0.f) _facing = (dir.x>0.f)? 1 : -1;
}

void Player::jump(){
    if(_onGround && _body){
        _body->setVelocity(Vec2(_body->getVelocity().x, 0.f));
        _body->applyImpulse(Vec2(0.f, 520.f));
        _onGround=false;
    }
}

void Player::fastFall(){
    if(_body) _body->applyImpulse(Vec2(0.f,-360.f));
}

void Player::shoot(){
    // Scene sẽ tạo Bullet qua contact/hàm tiện ích – ở Drop 1.1 tạm để Scene gọi trực tiếp
}

void Player::slash(){
    // tương tự – Scene sẽ spawn Slash hitbox
}

void Player::takeDamage(int dmg, const Vec2& knock){
    if(_iframe>0.f) return;
    _hp = std::max(0, _hp - dmg);
    if(_body){
        _body->setVelocity(Vec2::ZERO);
        _body->applyImpulse(knock);
    }
    // i-frame 1.0s + nhấp nháy
    _iframe = 1.0f;
    stopActionByTag(777);
    auto blink = RepeatForever::create(Sequence::create(FadeTo::create(0.06f,80),
                                                        FadeTo::create(0.06f,255), nullptr));
    blink->setTag(777); runAction(blink);
}

void Player::footBegin(){ _footContacts++; _onGround = _footContacts>0; }
void Player::footEnd(){ _footContacts = std::max(0, _footContacts-1); _onGround = _footContacts>0; }

void Player::update(float dt){
    if(!_body) return;
    auto v = _body->getVelocity();
    v.x = _moveDir.x * _speed;
    _body->setVelocity(v);

    if(_iframe>0.f){
        _iframe -= dt;
        if(_iframe<=0.f){ _iframe=0.f; stopActionByTag(777); setOpacity(255); }
    }
}
