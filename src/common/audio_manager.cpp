#include "audio_manager.h"
#include <iostream>

AudioManager::AudioManager() : isMusicPlaying(false) {}

AudioManager::~AudioManager() {
    stopMusic();
}

bool AudioManager::init() {
    // Initialize any global audio settings if necessary (for example, default device selection).
    return true;
}

bool AudioManager::playMusic(const std::string& filePath) {
    if (isMusicPlaying.load()) {
        std::cerr << "Music is already playing!" << std::endl;
        return false;
    }

    // Initialize decoder for the given file path
    ma_result result = ma_decoder_init_file(filePath.c_str(), nullptr, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize decoder for file: " << filePath << std::endl;
        return false;
    }

    // Configure playback device
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = audioCallback;
    deviceConfig.pUserData         = this;  // Pass the instance of AudioManager for callback usage

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

// Audio callback function to handle looping and playback
void AudioManager::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioManager* audioMgr = static_cast<AudioManager*>(pDevice->pUserData);  // Access the AudioManager instance
    ma_decoder* pDecoder = &audioMgr->decoder;

    if (!pDecoder) return;  // Safety check

    ma_uint64 framesRead;
    ma_result result = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    // Check if the end of the audio is reached and loop if necessary
    if (result == MA_SUCCESS && framesRead < frameCount) {
        // Loop by seeking back to the start of the file
        ma_decoder_seek_to_pcm_frame(pDecoder, 0);
        ma_decoder_read_pcm_frames(pDecoder, 
            static_cast<ma_uint8*>(pOutput) + (framesRead * ma_get_bytes_per_frame(pDecoder->outputFormat, pDecoder->outputChannels)), 
            frameCount - framesRead, nullptr);
    }

    (void)pInput;  // Suppress unused variable warning (as input is not needed in playback)
}
