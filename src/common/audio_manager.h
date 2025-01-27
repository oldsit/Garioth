#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#define MINI_AUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <string>
#include <atomic>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();
    bool playMusic(const std::string& filePath);
    void stopMusic();
    bool isPlaying() const;

private:
    ma_decoder decoder;
    ma_device device;
    std::atomic<bool> isMusicPlaying;  // Thread-safe flag

    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif
