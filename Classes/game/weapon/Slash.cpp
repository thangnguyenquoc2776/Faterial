#include "game/weapon/Slash.h"
#include "physics/PhysicsDefs.h"          // <-- CAT_SENSOR, MASK_SENSOR, ShapeTag::SLASH
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsShape.h"
#include "2d/CCDrawNode.h"
#include <algorithm>
#include <cmath>

using namespace cocos2d;

Slash* Slash::create(const Vec2& origin, float angleRad, float sizeOrRange, float durationSec) {
    auto p = new (std::nothrow) Slash();
    if (p && p->init()) {
        p->autorelease();

        // Tag gameplay + name để phân biệt trong onContact
        p->setTagEx(phys::Tag::SLASH);
        p->setName("player_slash");

        p->setPosition(origin);

        // (Optional) Visual debug — giữ nguyên tắt đi để gọn
        // auto dn = DrawNode::create();
        float r  = std::max(8.0f, sizeOrRange);

        Vec2 a( std::cos(angleRad), std::sin(angleRad) );
        Vec2 p0 = Vec2::ZERO;
        Vec2 p1 = a * r;
        Vec2 p2 = a.rotateByAngle(Vec2::ZERO, 0.35f) * (r * 0.75f);
        Vec2 tri[3] = { p0, p1, p2 };
        // dn->drawSolidPoly(tri, 3, Color4F(1.f, 1.f, 0.4f, 0.35f));
        // p->addChild(dn);

        // Body sensor hình hộp mỏng, đặt lệch về hướng tấn công
        auto body = PhysicsBody::create();
        body->setDynamic(false);
        body->setGravityEnable(false);
        body->setRotationEnable(false);

        // Hộp mỏng theo hướng chém (offset theo vector a)
        const float boxW = r * 0.9f;
        const float boxH = std::max(10.0f, r * 0.35f);
        Vec2 offset = a * (r * 0.45f);

        auto shape = PhysicsShapeBox::create(Size(boxW, boxH), PhysicsMaterial(0,0,0), offset);
        shape->setSensor(true);
        shape->setTag(static_cast<int>(phys::ShapeTag::SLASH));
        body->addShape(shape);

        // Masks: sensor chỉ cần contact (MASK_SENSOR = CAT_ALL) để bắt ENEMY và ENEMY_PROJ
        body->setCategoryBitmask(static_cast<int>(phys::CAT_SENSOR));
        body->setCollisionBitmask(0);
        body->setContactTestBitmask(static_cast<int>(phys::MASK_SENSOR));

        p->addComponent(body);

        // Xoay để visual khớp hướng
        p->setRotation(-CC_RADIANS_TO_DEGREES(angleRad));

        // Tự hủy sau durationSec
        p->runAction(Sequence::create(
            DelayTime::create(std::max(0.05f, durationSec)),
            CallFunc::create([p]{ p->removeFromParent(); }),
            nullptr
        ));
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}
