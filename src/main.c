#include <stdio.h>
#include <stdlib.h>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <whisper.h>
#include "loader.h"

// these are the labels where the model is linked
extern const unsigned char _binary_ggml_base_en_bin_start[];
extern const unsigned char _binary_ggml_base_en_bin_end[];

int main(void){
    printf("Hello, World!\n");

    size_t model_size = (size_t)(_binary_ggml_base_en_bin_end - _binary_ggml_base_en_bin_start);
    printf("Embedded model size: %zu MiB\n", model_size/(1024*1024));

    struct whisper_context* ctx = whisper_init_from_buffer(
        _binary_ggml_base_en_bin_start,
        model_size
    );
    if(ctx==NULL){
        fprintf(stderr, "Error: Failed to load embedded model\n");
        exit(1);
    }

    printf("Model loaded successfully from embedded memory\n");

    whisper_free(ctx);

    // test reading video
    size_t wav_size;
    unsigned char* wav_data = load_audio_via_ffmpeg("video.mp4",&wav_size);

    if(wav_data){
        drwav wav;
        if(drwav_init_memory(&wav, wav_data, wav_size, NULL)){
            printf("Successfully decoded WAV from memory!\n");
            printf("Sample rate: %d\n", wav.sampleRate);
            printf("Channels: %d\n", wav.channels);

            drwav_uninit(&wav);
        }
        free(wav_data);
    }
    return 0;
}