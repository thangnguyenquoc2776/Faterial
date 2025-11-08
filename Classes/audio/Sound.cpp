#include "audio/Sound.h"
#include "audio/include/AudioEngine.h"
#include "base/CCUserDefault.h"
#include "platform/CCFileUtils.h"
#include "base/ccMacros.h"
#include "cocos2d.h"
#include <string>

using namespace cocos2d;
using AE = cocos2d::AudioEngine;

namespace {
    int   s_bgmId  = -1;
    float s_music  = 0.8f;
    float s_sfx    = 0.8f;
    int   s_runId  = -1;
    std::string s_bgmFile;

    //! Cho phép chỉ đặt "gốc tên" (không đuôi) trong Resources/audio/
    static const char* kBgmMain = "audio/bgm_main";
    static const char* kSlash   = "audio/sfx_slash_hit";
    static const char* kSlash2  = "audio/sfx_slash_nothit";
    static const char* kShoot   = "audio/sfx_power_slash";
    static const char* kPickup  = "audio/sfx_pickup_coin";
    static const char* kRun     = "audio/sfx_run_boost";
    static const char* kJump    = "audio/jump";
    static const char* kStar    = "audio/collect_star";
    static const char* kUpgrade = "audio/pickup_upgrade";


    static std::string pickExisting(const std::string& stem) {
        static const char* exts[3] = { ".wav", ".mp3", ".ogg" };
        FileUtils* fu = FileUtils::getInstance();
        for (int i = 0; i < 3; ++i) {
            std::string p = stem + exts[i];
            if (fu->isFileExist(p)) return p;
        }
        CCLOG("SND missing: none of (%s.wav|.mp3|.ogg) exists", stem.c_str());
        return std::string();
    }

    static int play2dSafe(const std::string& file, bool loop, float vol) {
        if (file.empty()) return -1;
        int id = AE::play2d(file, loop, vol);
        if (id == -1) CCLOG("SND play2d failed: %s", file.c_str());
        return id;
    }
}

void snd::init() {
    UserDefault* ud = UserDefault::getInstance();
    s_music = clampf(ud->getFloatForKey("cfg.music", 0.8f), 0.f, 1.f);
    s_sfx   = clampf(ud->getFloatForKey("cfg.sfx",   0.8f), 0.f, 1.f);

    std::string p;
    p = pickExisting(kSlash);   if (!p.empty()) AE::preload(p);
    p = pickExisting(kShoot);   if (!p.empty()) AE::preload(p);
    p = pickExisting(kPickup);  if (!p.empty()) AE::preload(p);
    p = pickExisting(kBgmMain); if (!p.empty()) AE::preload(p);
    p = pickExisting(kRun);     if (!p.empty()) AE::preload(p);
    p = pickExisting(kJump);    if (!p.empty()) AE::preload(p);
    p = pickExisting(kSlash2);  if (!p.empty()) AE::preload(p);
    p = pickExisting(kStar);    if (!p.empty()) AE::preload(p);
    p = pickExisting(kUpgrade); if (!p.empty()) AE::preload(p);
}

void snd::setVolumes(float music, float sfx) {
    s_music = clampf(music, 0.f, 1.f);
    s_sfx   = clampf(sfx,   0.f, 1.f);
    if (s_bgmId != -1) AE::setVolume(s_bgmId, s_music);
}

float snd::musicVol() { return s_music; }
float snd::sfxVol()   { return s_sfx;   }

void snd::playBgm(const std::string& fileOrStem, bool loop) {
    std::string file = fileOrStem;

    FileUtils* fu = FileUtils::getInstance();
    if (!fu->isFileExist(file)) {
        if (file.find('.') == std::string::npos) {
            file = pickExisting(file);
        } else {
            std::string stem = file.substr(0, file.find_last_of('.'));
            file = pickExisting(stem);
        }
    }

    if (s_bgmId != -1) { AE::stop(s_bgmId); s_bgmId = -1; }
    s_bgmFile = file;
    s_bgmId   = play2dSafe(file, loop, s_music);
}

void snd::stopBgm() {
    if (s_bgmId != -1) { AE::stop(s_bgmId); s_bgmId = -1; }
}

void snd::sfxSlash() { play2dSafe(pickExisting(kSlash),  false, s_sfx); }
void snd::sfxShoot() { play2dSafe(pickExisting(kShoot),  false, s_sfx); }
void snd::sfxCoin()  { play2dSafe(pickExisting(kPickup), false, s_sfx); }
void snd::sfxJump()  { play2dSafe(pickExisting(kJump),   false, s_sfx); }
void snd::sfxSlash2()  { play2dSafe(pickExisting(kSlash2),   false, s_sfx); }
void snd::sfxStar()  { play2dSafe(pickExisting(kStar),   false, s_sfx); }
void snd::sfxUpgrade()  { play2dSafe(pickExisting(kUpgrade),   false, s_sfx); }

void snd::sfxRun() {
    if (s_runId != -1) return;
    s_runId = play2dSafe(pickExisting(kRun), true, s_sfx);
}

void snd::stopSfxRun() {
    if (s_runId != -1) {
        AE::stop(s_runId);
        s_runId = -1;
    }
}

