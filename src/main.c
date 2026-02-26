#include <stdio.h>
#include <stdlib.h>
#define DR_WAV_IMPLEMENTATION
#include <inttypes.h>

#include "dr_wav.h"
#include <whisper.h>
#include "loader.h"

// these are the labels where the model is linked
extern const unsigned char _binary_ggml_base_en_bin_start[];
extern const unsigned char _binary_ggml_base_en_bin_end[];

void whisper_log_cb(struct whisper_context *ctx, struct whisper_state *state, int n_new, void* user_data);

int main(void){
    // test reading video
    size_t wav_size;
    unsigned char* wav_data = load_audio_via_ffmpeg("video.mp4",&wav_size);

    if(!wav_data){
        fprintf(stderr, "MAIN: Failed to load audio\n");
        exit(1);
    }

    // load audio data as float array
    drwav wav;
    if(!drwav_init_memory(&wav, wav_data, wav_size, NULL)){
        fprintf(stderr, "MAIN: Failed to initialize drwav\n");
        exit(1);
    }

    printf("Successfully decoded WAV from memory!\n");
    printf("Sample rate: %d\n", wav.sampleRate);
    printf("Channels: %d\n", wav.channels); // should be 1
    if(wav.channels != 1){
        fprintf(stderr, "MAIN: WAV channels must be 1\n");
        exit(1);
    }

    float* pDecodedInterleavedPCMFrames = malloc(wav.totalPCMFrameCount * sizeof(float));
    drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pDecodedInterleavedPCMFrames);

    // load model
    size_t model_size = (size_t)(_binary_ggml_base_en_bin_end - _binary_ggml_base_en_bin_start);
    printf("Embedded model size: %zu MiB\n", model_size/(1024*1024));

    struct whisper_context* ctx = whisper_init_from_buffer(
        _binary_ggml_base_en_bin_start,
        model_size
    );
    if(ctx==NULL){
        fprintf(stderr, "MAIN: Failed to load embedded model\n");
        exit(1);
    }

    printf("Model loaded successfully from embedded memory\n");

    // transcribe audio
    // setup params
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.new_segment_callback = whisper_log_cb;
    wparams.new_segment_callback_user_data = NULL;
    wparams.n_threads = 8;
    wparams.language = "en";

    // perform inference
    printf("BEGIN TRANSCRIPT\n");
    if(whisper_full(
        ctx,
        wparams,
        pDecodedInterleavedPCMFrames,
        wav.totalPCMFrameCount
    ) != 0){
        return 1;
    }

    whisper_free(ctx);
    drwav_uninit(&wav);
    free(wav_data);
    return 0;
}

void whisper_log_cb(struct whisper_context *ctx, struct whisper_state *state, int n_new, void* user_data){
    const int n_segments = whisper_full_n_segments(ctx);
    const int i = n_segments - 1;

    if(i>=0){
        const char * text = whisper_full_get_segment_text(ctx, i);
        int64_t t0 = whisper_full_get_segment_t0(ctx, i);
        int64_t t1 = whisper_full_get_segment_t1(ctx, i);

        // format as SRT later
        printf("[%07ld --> %07ld] %s\n", t0, t1, text);
        fflush(stdout);
    }
}
