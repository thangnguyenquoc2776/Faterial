#include "game/Player.h"
#include "physics/PhysicsDefs.h"
#include "game/Bullet.h"
#include "game/SlashHitbox.h"
USING_NS_CC;

Player* Player::create(){
    auto p = new(std::nothrow) Player();
    if(p && p->init()){ p->autorelease(); return p; }
    CC_SAFE_DELETE(p); return nullptr;
}

bool Player::init(){
    if(!Sprite::init()) return false;
    setTextureRect(Rect(0.f, 0.f, 40.f, 56.f));
    setColor(Color3B::BLUE);

    auto dn = DrawNode::create();
    Vec2 r[4] = { Vec2(-20.f,-28.f), Vec2(20.f,-28.f), Vec2(20.f,28.f), Vec2(-20.f,28.f) };
    dn->drawPoly(r, 4, true, Color4F::WHITE);
    addChild(dn);

    scheduleUpdate();
    return true;
}

void Player::enablePhysics(const Vec2& pos){
    if(_body) return;
    _body = PhysicsBody::createBox(Size(40.f,56.f), PhysicsMaterial(0.1f,0.f,0.9f));
    _body->setDynamic(true);
    _body->setRotationEnable(false);
    _body->setCategoryBitmask(phys::CAT_PLAYER);
    _body->setCollisionBitmask(phys::CAT_WORLD|phys::CAT_ENEMY|phys::CAT_ITEM|phys::CAT_GATE|phys::CAT_CRATE);
    _body->setContactTestBitmask(phys::all());
    setPhysicsBody(_body);
    setPosition(pos);
}

void Player::setMoveDir(const Vec2& dir){
    _moveDir = dir;
    if(dir.x > 0.f) _facingRight = true;
    else if(dir.x < 0.f) _facingRight = false;
}

void Player::jump(){
    if(_onGround && _body){
        _body->setVelocity(Vec2(_body->getVelocity().x, 0.f));
        _body->applyImpulse(Vec2(0.f, 500.f));
        _onGround = false;
    }
}


void Player::_syncGroundState(){
    if(!_body) return;
    _onGround = std::abs(_body->getVelocity().y) < 0.1f;
}

void Player::shoot(){
    if(_cdShoot>0.f) return;
    _cdShoot = 0.18f;
    Vec2 dir = _facingRight ? Vec2(1.f,0.f) : Vec2(-1.f,0.f);
    auto b = Bullet::create(dir, 700.f, 1.6f);
    b->setPosition(getPosition() + Vec2(_facingRight?22.f:-22.f, 8.f));
    getParent()->addChild(b, 5);
}

void Player::slash(){
    if(_cdSlash>0.f) return;
    _cdSlash = 0.25f;
    Size box(46.f, 30.f);
    auto h = SlashHitbox::create(box, 0.12f);
    h->setTag((int)phys::Tag::SLASH);
    h->setPosition(getPosition() + Vec2(_facingRight?34.f:-34.f, 6.f));
    getParent()->addChild(h, 5);
}

void Player::update(float dt){
    if(_cdShoot>0.f) _cdShoot -= dt;
    if(_cdSlash>0.f) _cdSlash -= dt;

    if(!_body) return;
    auto v = _body->getVelocity();
    v.x = _moveDir.x * _speed;
    _body->setVelocity(v);
    _syncGroundState();
}
