#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include <chrono>
#include <iostream>

void DataCallback(ma_device* p_device, void* p_output, const void* p_input,
                  ma_uint32 frame_count) {
    // In playback mode copy data to pOutput. In capture mode read data from
    // pInput. In full-duplex mode, both pOutput and pInput will be valid and
    // you can move data from pInput into pOutput. Never process more than
    // frameCount frames.
    ma_decoder* p_decoder = static_cast<ma_decoder*>(p_device->pUserData);
    if (p_decoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(p_decoder, p_output, frame_count, NULL);

    (void)p_input;
}

ma_bool32 PlaySound(const char* audio_file, ma_decoder* decoder,
                    ma_device* device) {
    ma_result result = ma_decoder_init_file(audio_file, NULL, decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Unable to initialize the decoder with an audio file!"
                  << std::endl;
        return result;
    }

    ma_device_config device_config =
        ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = decoder->outputFormat;
    device_config.playback.channels = decoder->outputChannels;
    device_config.sampleRate = decoder->outputSampleRate;
    device_config.dataCallback = DataCallback;
    device_config.pUserData = decoder;

    result = ma_device_init(NULL, &device_config, device);
    if (result != MA_SUCCESS) {
        std::cerr << "Unable to initialize the device!" << std::endl;
        ma_decoder_uninit(decoder);
        return result;
    }

    result = ma_device_start(device);
    if (result != MA_SUCCESS) {
        std::cerr << "Unable to start the device!" << std::endl;
        ma_device_uninit(device);
        ma_decoder_uninit(decoder);
        return result;
    }

    return MA_SUCCESS;
}

int main() {
    const char* audio_file = "audio/test.mp3";
    ma_decoder decoder;
    ma_device device;

    std::cout << "----------------------------------------------------"
              << std::endl;
    std::cout << "| Play mp3 file with miniaudio via low level apis. |"
              << std::endl;
    std::cout << "----------------------------------------------------"
              << std::endl;

    PlaySound(audio_file, &decoder, &device);

    auto start = std::chrono::steady_clock::now();

    // Game loop
    while (true) {
        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;
        auto duration = std::chrono::duration<double, std::milli>(diff).count();
        if (duration >= 8000) {
            ma_device_stop(&device);
            PlaySound(audio_file, &decoder, &device);
            start = std::chrono::steady_clock::now();
        }
    }

    // Free resources
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}
