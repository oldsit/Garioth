#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

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

    void setVolume(float volume);  // New method to adjust volume

private:
    ma_decoder decoder;
    ma_device device;
    std::atomic<bool> isMusicPlaying;
    float volume;  // Stores the volume level (0.0 to 1.0)

    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif  // AUDIO_MANAGER_H
