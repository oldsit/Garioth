#include "audio_manager.h"
#include <iostream>

AudioManager::AudioManager() : isMusicPlaying(false) {}

AudioManager::~AudioManager() {
    stopMusic();
}

bool AudioManager::init() {
    // Potentially add initialization logic here (e.g., global audio context)
    return true;
}

bool AudioManager::playMusic(const std::string& filePath) {
    if (isMusicPlaying.load()) {
        std::cerr << "Music is already playing!" << std::endl;
        return false;
    }

    ma_result result = ma_decoder_init_file(filePath.c_str(), nullptr, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize decoder for file: " << filePath << std::endl;
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = audioCallback;
    deviceConfig.pUserData         = &decoder;

    result = ma_device_init(nullptr, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device!" << std::endl;
        ma_decoder_uninit(&decoder);
        return false;
    }

    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start playback device!" << std::endl;
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return false;
    }

    isMusicPlaying.store(true);
    return true;
}

void AudioManager::stopMusic() {
    if (isMusicPlaying.load()) {
        ma_device_stop(&device);
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        isMusicPlaying.store(false);
    }
}

bool AudioManager::isPlaying() const {
    return isMusicPlaying.load();
}

// Audio callback function to feed audio data to the device
void AudioManager::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_decoder* pDecoder = static_cast<ma_decoder*>(pDevice->pUserData);
    if (pDecoder) {
        ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, nullptr);
    }
    (void)pInput;  // Suppress unused variable warning
}
