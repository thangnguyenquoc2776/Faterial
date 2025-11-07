#pragma once
#include "base/CCUserDefault.h"

struct Settings {
    float musicVolume = 0.8f; // 0..1
    float sfxVolume   = 1.0f; // 0..1

    static Settings load() {
        Settings s;
        auto* ud = cocos2d::UserDefault::getInstance();
        s.musicVolume = ud->getFloatForKey("musicVolume", 0.8f);
        s.sfxVolume   = ud->getFloatForKey("sfxVolume",   1.0f);
        return s;
    }
    void save() const {
        auto* ud = cocos2d::UserDefault::getInstance();
        ud->setFloatForKey("musicVolume", musicVolume);
        ud->setFloatForKey("sfxVolume",   sfxVolume);
        ud->flush();
    }
};
