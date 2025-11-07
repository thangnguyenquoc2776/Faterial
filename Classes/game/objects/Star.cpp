// Star.cpp
#include "game/objects/Star.h"
#include "physics/PhysicsDefs.h"
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"
USING_NS_CC;
Star* Star::create(){ auto p=new(std::nothrow) Star(); if(p && p->init()){p->autorelease(); return p;} CC_SAFE_DELETE(p); return nullptr; }
bool Star::init(){
    if(!Entity::init()) return false;
    setTagEx(phys::Tag::STAR);


    _sprite = Sprite::create("sprites/objects/star/star_1.png"); // frame đầu tiên
    _sprite->setScale(1.7f); // tùy chỉnh kích cỡ hiển thị
    addChild(_sprite, 1);

    Vector<SpriteFrame*> frames;
    auto firstSize = _sprite->getContentSize();
    for (int i = 1; i <= 13; i++) { // giả sử bạn có star_1.png -> star_5.png
        std::string framePath = StringUtils::format("sprites/objects/star/star_%d.png", i);
        auto frame = SpriteFrame::create(framePath, Rect(0, 0, firstSize.width, firstSize.height));
        frame->setOriginalSize(firstSize);
        frames.pushBack(frame);
    }

    auto animation = Animation::createWithSpriteFrames(frames, 0.085f); // 0.085s mỗi frame
    auto animate = Animate::create(animation);
    _sprite->runAction(RepeatForever::create(animate));


    auto body = PhysicsBody::createCircle(12);
    body->setDynamic(false);
    body->setCategoryBitmask(phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask(phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}
