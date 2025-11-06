#pragma once
#include "game/Entity.h"
#include "2d/CCSprite.h"

class Upgrade : public Entity {
public:
    enum class Type {
        SPEED = 0, JUMP = 1, DAMAGE = 2, BULLET = 3,
        RANGE = 4, DOUBLEJUMP = 5, EXTRA_LIFE = 6
    };

    static Upgrade* create();               // random theo bảng loại, nhưng thời lượng cố định
    static Upgrade* create(Type t);         // tạo đúng loại, tự gán duration chuẩn
    static Upgrade* createRandom();         // pick loại, gán duration chuẩn

    bool init() override;

    Type  type()     const { return _type; }
    float duration() const { return _duration; }   // EXTRA_LIFE => 3s chỉ để HUD báo
    static float durationFor(Type t);              // <— HÀM CHUẨN HOÁ DUY NHẤT

    void  set(Type t);                             // gán loại + duration chuẩn

private:
    cocos2d::Sprite* _sprite = nullptr;
    Type   _type      = Type::SPEED;
    float  _duration  = 0.f;

    void   _refreshVisual();
};
