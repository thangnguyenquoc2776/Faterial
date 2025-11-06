#pragma once
#include "game/Entity.h"

class Chest : public Entity {
public:
    CREATE_FUNC(Chest);
    bool init() override;

    // Mở rương: nhả loot xong tự hủy
    void open();

private:
    bool _opened = false;
};
