// Entity.h
#pragma once
#include "cocos2d.h"
#include "physics/PhysicsDefs.h"

class Entity : public cocos2d::Node {
public:
    CREATE_FUNC(Entity);
    bool init() override { return Node::init(); }
    void setTagEx(phys::Tag t){ _gtag = t; setTag(static_cast<int>(t)); }
    phys::Tag getTagEx() const { return _gtag; }
protected:
    phys::Tag _gtag = phys::Tag::NONE;
};
