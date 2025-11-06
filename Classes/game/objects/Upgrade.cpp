#include "game/objects/Upgrade.h"
#include "physics/PhysicsDefs.h"
#include "base/ccRandom.h"
USING_NS_CC;

// ======= BẢNG THỜI LƯỢNG CỐ ĐỊNH (đổi 1 chỗ là xong) =======
static constexpr float DUR_SPEED      = 45.f;
static constexpr float DUR_JUMP       = 45.f;
static constexpr float DUR_DAMAGE     = 30.f;
static constexpr float DUR_BULLET     = 40.f;
static constexpr float DUR_RANGE      = 35.f;
static constexpr float DUR_DOUBLEJUMP = 60.f;
static constexpr float DUR_EXTRALIFE  = 3.f;  // chỉ để HUD hiển thị
// ===========================================================

float Upgrade::durationFor(Upgrade::Type t){
    switch(t){
        case Type::SPEED:      return DUR_SPEED;
        case Type::JUMP:       return DUR_JUMP;
        case Type::DAMAGE:     return DUR_DAMAGE;
        case Type::BULLET:     return DUR_BULLET;
        case Type::RANGE:      return DUR_RANGE;
        case Type::DOUBLEJUMP: return DUR_DOUBLEJUMP;
        case Type::EXTRA_LIFE: return DUR_EXTRALIFE;
        default: return 30.f;
    }
}

Upgrade* Upgrade::create(){ return createRandom(); }

Upgrade* Upgrade::create(Upgrade::Type t){
    auto p = new(std::nothrow) Upgrade();
    if (p && p->init()){
        p->autorelease();
        p->set(t);                           // <— luôn gán duration chuẩn
        return p;
    }
    CC_SAFE_DELETE(p); return nullptr;
}

Upgrade* Upgrade::createRandom(){
    // Tỉ lệ loại — thời lượng sẽ tự lấy từ durationFor()
    int roll = RandomHelper::random_int(1,100);
    Type t;
    if      (roll<=18) t=Type::SPEED;
    else if (roll<=36) t=Type::JUMP;
    else if (roll<=54) t=Type::DAMAGE;
    else if (roll<=70) t=Type::BULLET;
    else if (roll<=82) t=Type::RANGE;
    else if (roll<=92) t=Type::DOUBLEJUMP;
    else               t=Type::EXTRA_LIFE;
    return create(t);
}

bool Upgrade::init(){
    if(!Entity::init()) return false;
    setTagEx((int)phys::Tag::UPGRADE);

    _sprite = Sprite::create("sprites/objects/upgrade/upgrade_generic.png");
    if(!_sprite){ _sprite = Sprite::create(); _sprite->setTextureRect(Rect(0,0,20,20)); }
    _sprite->setScale(1.2f);
    addChild(_sprite);

    auto body = PhysicsBody::createCircle(12);
    body->setDynamic(false);
    body->setCategoryBitmask((int)phys::CAT_ITEM);
    body->setCollisionBitmask(0);
    body->setContactTestBitmask((int)phys::CAT_PLAYER);
    setPhysicsBody(body);
    return true;
}

void Upgrade::set(Type t){
    _type = t;
    _duration = durationFor(t);     // <— Ở ĐÂY: chốt duration
    _refreshVisual();
}

void Upgrade::_refreshVisual(){
    const char* name = "upgrade_generic.png";
    switch(_type){
        case Type::SPEED:      name="upgrade_speed.png"; break;
        case Type::JUMP:       name="upgrade_jump.png"; break;
        case Type::DAMAGE:     name="upgrade_damage.png"; break;
        case Type::BULLET:     name="upgrade_bullet.png"; break;
        case Type::RANGE:      name="upgrade_range.png"; break;
        case Type::DOUBLEJUMP: name="upgrade_doublejump.png"; break;
        case Type::EXTRA_LIFE: name="upgrade_extralife.png"; break;
    }
    auto path = std::string("sprites/objects/upgrade/") + name;
    if (FileUtils::getInstance()->isFileExist(path)) _sprite->setTexture(path);
}
