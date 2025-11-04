// Classes/game/Entity.h
#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

// Alias toàn cục để code cũ dùng Tag::XYZ vẫn hợp lệ
using Tag = phys::Tag;

class Entity : public cocos2d::Node {
public:
    CREATE_FUNC(Entity);

    // Node::init() mặc định
    bool init() override { return cocos2d::Node::init(); }

    // Tag gameplay mở rộng (không đụng tới Node::setTag(int) của cocos2d)
    void setTagEx(Tag t)           { _gtag = t; }
    void setTagExInt(int t)        { _gtag = static_cast<Tag>(t); }  // back-compat
    void setTagEx(int t)           { _gtag = static_cast<Tag>(t); }  // back-compat: setTagEx(1)
    Tag  getTagEx() const          { return _gtag; }

protected:
    Tag _gtag = Tag::NONE;
};
