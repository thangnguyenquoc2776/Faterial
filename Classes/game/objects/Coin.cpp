// Coin.cpp
#include "game/objects/Coin.h"
#include "physics/PhysicsDefs.h"
USING_NS_CC;
Coin* Coin::create(){ auto p=new(std::nothrow) Coin(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Coin::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::COIN);

    _sprite = Sprite::create("sprites/objects/coin/coin_1.png"); // frame đầu tiên
    _sprite->setScale(1.5f); // tùy chỉnh kích cỡ hiển thị
    addChild(_sprite, 1);

    Vector<SpriteFrame*> frames;
    auto firstSize = _sprite->getContentSize();
    for (int i = 1; i <= 4; i++) { // giả sử bạn có coin_1.png -> coin_5.png
        std::string framePath = StringUtils::format("sprites/objects/coin/coin_%d.png", i);
        auto frame = SpriteFrame::create(framePath, Rect(0, 0, firstSize.width, firstSize.height));
        frame->setOriginalSize(firstSize);
        frames.pushBack(frame);
    }

    auto animation = Animation::createWithSpriteFrames(frames, 0.2f); // 0.4s mỗi frame
    auto animate = Animate::create(animation);
    _sprite->runAction(RepeatForever::create(animate));

    // auto dn = DrawNode::create();
    // dn->drawSolidCircle(Vec2::ZERO, 10, 0, 24, Color4F(1,0.85f,0,1));
    // addChild(dn);
    auto body = PhysicsBody::createCircle(10);
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_ITEM);
    body->setCollisionBitmask(0);               // sensor-only
    body->setContactTestBitmask(phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}
