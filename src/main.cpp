#define MA_NO_VORBIS
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
#include <miniaudio/extras/miniaudio_libvorbis.h>

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

static ma_result MaDecodingBackendInitLibvorbis(
    void* p_user_data, ma_read_proc on_read, ma_seek_proc on_seek,
    ma_tell_proc on_tell, void* p_read_seek_tell_user_data,
    const ma_decoding_backend_config* p_config,
    const ma_allocation_callbacks* p_allocation_callbacks,
    ma_data_source** pp_backend) {
    ma_result result;
    ma_libvorbis* p_vorbis;

    (void)p_user_data;

    p_vorbis = static_cast<ma_libvorbis*>(
        ma_malloc(sizeof(*p_vorbis), p_allocation_callbacks));
    if (p_vorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result =
        ma_libvorbis_init(on_read, on_seek, on_tell, p_read_seek_tell_user_data,
                          p_config, p_allocation_callbacks, p_vorbis);
    if (result != MA_SUCCESS) {
        ma_free(p_vorbis, p_allocation_callbacks);
        return result;
    }

    *pp_backend = p_vorbis;

    return MA_SUCCESS;
}

static ma_result MaDecodingBackendInitFileLibvorbis(
    void* p_user_data, const char* p_file_path,
    const ma_decoding_backend_config* p_config,
    const ma_allocation_callbacks* p_allocation_callbacks,
    ma_data_source** pp_backend) {
    ma_result result;
    ma_libvorbis* p_vorbis;

    (void)p_user_data;

    p_vorbis = static_cast<ma_libvorbis*>(
        ma_malloc(sizeof(*p_vorbis), p_allocation_callbacks));
    if (p_vorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init_file(p_file_path, p_config,
                                    p_allocation_callbacks, p_vorbis);
    if (result != MA_SUCCESS) {
        ma_free(p_vorbis, p_allocation_callbacks);
        return result;
    }

    *pp_backend = p_vorbis;

    return MA_SUCCESS;
}

static void MaDecodingBackendUninitLibvorbis(
    void* p_user_data, ma_data_source* p_backend,
    const ma_allocation_callbacks* p_allocation_callbacks) {
    ma_libvorbis* p_vorbis = static_cast<ma_libvorbis*>(p_backend);

    (void)p_user_data;

    ma_libvorbis_uninit(p_vorbis, p_allocation_callbacks);
    ma_free(p_vorbis, p_allocation_callbacks);
}

ma_bool32 PlaySound(const char* audio_file, ma_decoder* decoder,
                    ma_device* device) {
    static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libvorbis = {
        MaDecodingBackendInitLibvorbis, MaDecodingBackendInitFileLibvorbis,
        NULL, /* onInitFileW() */
        NULL, /* onInitMemory() */
        MaDecodingBackendUninitLibvorbis};

    ma_decoding_backend_vtable* p_custom_backend_v_tables[] = {
        &g_ma_decoding_backend_vtable_libvorbis};

    ma_decoder_config decoder_config;
    decoder_config = ma_decoder_config_init_default();
    decoder_config.pCustomBackendUserData = NULL;
    decoder_config.ppCustomBackendVTables = p_custom_backend_v_tables;
    decoder_config.customBackendCount = sizeof(p_custom_backend_v_tables) /
                                        sizeof(p_custom_backend_v_tables[0]);

    ma_result result =
        ma_decoder_init_file(audio_file, &decoder_config, decoder);
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
    const char* audio_file = "audio/test.ogg";
    ma_decoder decoder;
    ma_device device;

    std::cout << "----------------------------------------------------"
              << std::endl;
    std::cout << "| Play ogg file with miniaudio via low level apis. |"
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
