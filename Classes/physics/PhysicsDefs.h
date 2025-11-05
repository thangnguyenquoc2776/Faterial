// Classes/physics/PhysicsDefs.h
#pragma once
#include <cstdint>
#include <initializer_list>
#include "cocos2d.h"

namespace phys {

// ==============================
// Bit/Mask cơ bản
// ==============================
using Mask = std::uint32_t;

enum : Mask {
    CAT_NONE   = 0u,
    CAT_WORLD  = 1u << 0,  // ground/map/platform
    CAT_PLAYER = 1u << 1,
    CAT_ENEMY  = 1u << 2,
    CAT_ITEM   = 1u << 3,  // coin/star/upgrade…
    CAT_BULLET = 1u << 4,  // projectile
    CAT_SENSOR = 1u << 5,  // generic sensors
    CAT_CRATE  = 1u << 6,
    CAT_GATE   = 1u << 7,
    CAT_SOLID  = 1u << 8,  // tường/vật thể rắn khác
    CAT_ALL    = 0xFFFFFFFFu
};

// ==============================
// Tag gameplay cho Entity
// ==============================
enum class Tag : int {
    NONE    = 0,
    COIN    = 1,
    STAR    = 2,
    UPGRADE = 3,
    GATE    = 4,
    CRATE   = 5,
    BULLET  = 6,
    SLASH   = 7,
    PLATE   = 8,
    PLAYER  = 9,
    ENEMY   = 10
};

// ==============================
// Tag cho PhysicsShape (sensor/fixture)
// ==============================
enum class ShapeTag : int {
    NONE  = 0,
    FOOT  = 1,  // chân player để bắt tiếp đất
    SLASH = 2   // hitbox chém
};

// Alias số nguyên để code cũ từng dùng phys::FOOT/SLASH vẫn chạy (C++14-safe).
constexpr int FOOT  = static_cast<int>(ShapeTag::FOOT);
constexpr int SLASH = static_cast<int>(ShapeTag::SLASH);

// ==============================
// Masks giữ lại cho back-compat
// ==============================
constexpr Mask MASK_BULLET = (CAT_WORLD | CAT_ENEMY | CAT_GATE | CAT_CRATE);
constexpr Mask MASK_SENSOR = CAT_ALL;

// ==============================
// Helpers cho mask
// ==============================
inline Mask all() { return CAT_ALL; }  // all() = mọi bit

// all({ A, B, C }) — C++14, không dùng fold-expression
inline Mask all(std::initializer_list<Mask> xs) {
    Mask m = 0u;
    for (Mask v : xs) m |= v;
    return m;
}

inline bool any(Mask m, Mask flags) { return (m & flags) != 0u; }

// ==============================
// Helpers thao tác với PhysicsBody / Node
// ==============================
inline bool hasCat(const cocos2d::PhysicsBody* body, Mask cat) {
    return body && ((static_cast<Mask>(body->getCategoryBitmask()) & cat) != 0u);
}
inline bool hasCat(const cocos2d::Node* node, Mask cat) {
    return node && hasCat(node->getPhysicsBody(), cat);
}

inline void setMasks(cocos2d::PhysicsBody* body, Mask category, Mask collide, Mask contact) {
    if (!body) return;
    body->setCategoryBitmask(static_cast<int>(category));
    body->setCollisionBitmask(static_cast<int>(collide));
    body->setContactTestBitmask(static_cast<int>(contact));
}
inline void setMasks(cocos2d::Node* node, Mask category, Mask collide, Mask contact) {
    if (!node) return;
    setMasks(node->getPhysicsBody(), category, collide, contact);
}

// đặt TÂM thân ở ngay phía trên mép trên topY, có skin nhỏ để không "lún"
inline float yOnTop(float topY, float bodyH, float skin = 0.5f) {
    return topY + bodyH * 0.5f + skin;
}


} // namespace phys

// ------------------------------------------------------------
// Legacy forwarders: cho những chỗ gọi all(...) không có tiền tố phys::
// ------------------------------------------------------------
inline phys::Mask all() { return phys::all(); }
inline phys::Mask all(std::initializer_list<phys::Mask> xs) { return phys::all(xs); }
