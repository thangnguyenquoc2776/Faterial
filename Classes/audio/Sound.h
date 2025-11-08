#pragma once
#include <string>

namespace snd {
    // Gọi 1 lần ở App khởi động
    void init();

    // Áp dụng volume (0..1), tự lấy từ UserDefault nếu bạn muốn
    void setVolumes(float music, float sfx);
    float musicVol();
    float sfxVol();

    // Nhạc nền
    void playBgm(const std::string& file, bool loop = true);
    void stopBgm();

    // Hiệu ứng
    void sfxSlash();
    void sfxShoot();
    void sfxCoin();
    void sfxRun();
    void stopSfxRun();
    void sfxJump();
    void sfxSlash2();
    void sfxStar();
    void sfxUpgrade();

}
