#include "game/weapon/Bullet.h"
#include "physics/PhysicsDefs.h"          // <-- cần để dùng phys::CAT_*, MASK_*, Tag
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include "2d/CCDrawNode.h"
#include <algorithm>                       // <-- std::max
#include "2d/CCSprite.h"
#include "2d/CCAnimation.h"
#include "2d/CCAnimationCache.h"

using namespace cocos2d;

Bullet* Bullet::create(const Vec2& origin, const Vec2& velocity, float lifeSec) {
    auto p = new (std::nothrow) Bullet();
    if (p && p->init()) {
        p->autorelease();

        // Tag gameplay + name cho dễ debug / lọc va chạm
        p->setTagEx(phys::Tag::BULLET);
        p->setName("player_proj");

        // Vị trí ban đầu
        p->setPosition(origin);

        // Sprite hiển thị
        p->_sprite = Sprite::create("sprites/wind/wind_spell_1.png"); // frame đầu tiên
        p->_sprite->setScale(2.5f); // tùy chỉnh kích cỡ hiển thị
        p->_sprite->setPositionY(0.f);
        p->addChild(p->_sprite, 1);

        if (velocity.x < 0)
            p->_sprite->setFlippedX(true);
        else
            p->_sprite->setFlippedX(false);


        // animation
        Vector<SpriteFrame*> frames;
        auto firstSize = p->_sprite->getContentSize();
        for (int i = 1; i <= 6; i++) {
            std::string framePath = StringUtils::format("sprites/wind/wind_spell_%d.png", i);
            auto frame = SpriteFrame::create(framePath, Rect(0, 0, firstSize.width, firstSize.height));
            frame->setOriginalSize(firstSize);
            frames.pushBack(frame);
        }
        auto animation = Animation::createWithSpriteFrames(frames, 0.1f);
        auto animate = Animate::create(animation);
        p->_sprite->runAction(RepeatForever::create(animate));


        // Thân vật lý
        auto body = PhysicsBody::createCircle(4.5f, PhysicsMaterial(0,0,0));
        body->setDynamic(true);
        body->setGravityEnable(false);
        body->setRotationEnable(false);

        // Masks: để đạn người chơi có thể "đụng" ENEMY + ENEMY_PROJ (đã có trong MASK_BULLET)
        body->setCategoryBitmask(static_cast<int>(phys::CAT_BULLET));
        body->setCollisionBitmask(static_cast<int>(phys::MASK_BULLET));
        body->setContactTestBitmask(static_cast<int>(phys::MASK_BULLET));

        // Gắn tag lên shape để contact listener nhận diện nhanh
        if (!body->getShapes().empty())
            body->getShapes().front()->setTag(static_cast<int>(phys::Tag::BULLET));

        p->addComponent(body);

        // Bay theo vận tốc đầu vào
        body->setVelocity(velocity);

        // Tự hủy sau lifeSec
        p->runAction(Sequence::create(
            DelayTime::create(std::max(0.05f, lifeSec)),
            CallFunc::create([p]{ p->removeFromParent(); }),
            nullptr
        ));
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}
