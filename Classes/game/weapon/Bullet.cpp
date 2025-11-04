#include "game/weapon/Bullet.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include "2d/CCDrawNode.h"

using namespace cocos2d;

Bullet* Bullet::create(const Vec2& origin, const Vec2& velocity, float lifeSec) {
    auto p = new (std::nothrow) Bullet();
    if (p && p->init()) {
        p->autorelease();

        // Tag gameplay
        p->setTagEx(phys::Tag::BULLET);

        // Vị trí ban đầu
        p->setPosition(origin);

        // Vẽ dấu chấm nhỏ cho dễ nhìn
        auto dot = DrawNode::create();
        dot->drawSolidCircle(Vec2::ZERO, 4.5f, 0, 12, Color4F(1.f, 0.95f, 0.2f, 1.f));
        p->addChild(dot);

        // Thân vật lý
        auto body = PhysicsBody::createCircle(4.5f, PhysicsMaterial(0,0,0));
        body->setDynamic(true);
        body->setGravityEnable(false);
        body->setRotationEnable(false);

        // Masks theo PhysicsDefs.h (C++14-safe)
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
