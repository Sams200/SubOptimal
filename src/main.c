#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#define DR_WAV_IMPLEMENTATION
#include <inttypes.h>

#include "dr_wav.h"
#include <whisper.h>
#include "loader.h"
#include "transcribe.h"

void whisper_log_cb(struct whisper_context *ctx, struct whisper_state *state, int n_new, void* user_data);

// suppress whisper.cpp output
static void whisper_silent_log_cb(enum ggml_log_level level, const char * text, void * user_data) {
    (void)level;
    (void)text;
    (void)user_data;
}

int main(int argc, char *argv[]){
    whisper_log_set(whisper_silent_log_cb, NULL);

    parse_args(argc, argv);
    char model_path[PATH_MAX];
    if(ensure_model(arguments.model, model_path, PATH_MAX)){
        exit(1);
    }

    printf("Using model: %s\n", arguments.model);
    printf("Source file: %s\n", arguments.source);
    printf("Output file: %s\n", arguments.output);
    // test reading video
    size_t wav_size;
    unsigned char* wav_data = load_audio_via_ffmpeg(arguments.source,&wav_size);

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
    struct whisper_context* ctx = whisper_init_from_file(
        model_path
    );
    if(ctx==NULL){
        fprintf(stderr, "MAIN: Failed to load embedded model\n");
        exit(1);
    }

    printf("Model loaded successfully from embedded memory\n");

    // open output file for writing
    FILE *output_file = fopen(arguments.output, "w");
    if (!output_file) {
        fprintf(stderr, "MAIN: Failed to open output file %s\n", arguments.output);
        exit(1);
    }

    // transcribe audio
    // setup params
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.new_segment_callback = whisper_log_cb;
    wparams.new_segment_callback_user_data = output_file;
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

    fclose(output_file);
    whisper_free(ctx);
    drwav_uninit(&wav);
    free(wav_data);
    return 0;
}

// Convert to SRT timestamp format (HH:MM:SS,mmm)
static void format_srt_timestamp(int64_t centiseconds, char* buffer){
    int64_t total_ms = centiseconds * 10;
    int64_t ms = total_ms % 1000;
    int64_t total_seconds = total_ms / 1000;
    int64_t seconds = total_seconds % 60;
    int64_t total_minutes = total_seconds / 60;
    int64_t minutes = total_minutes % 60;
    int64_t hours = total_minutes / 60;

    sprintf(buffer, "%02lld:%02lld:%02lld,%03lld",
            (long long)hours, (long long)minutes, (long long)seconds, (long long)ms);
}

void whisper_log_cb(struct whisper_context *ctx, struct whisper_state *state, int n_new, void* user_data){
    FILE *output_file = (FILE *)user_data;
    const int n_segments = whisper_full_n_segments(ctx);
    const int i = n_segments - 1;

    if(i>=0){
        const char * text = whisper_full_get_segment_text(ctx, i);
        int64_t t0 = whisper_full_get_segment_t0(ctx, i);
        int64_t t1 = whisper_full_get_segment_t1(ctx, i);

        char t0_str[32], t1_str[32];
        format_srt_timestamp(t0, t0_str);
        format_srt_timestamp(t1, t1_str);

        // format as SRT: segment_number, timestamp, text, blank line
        fprintf(stdout, "[%s --> %s] %s\n", t0_str, t1_str, text);
        fprintf(output_file, "%d\n%s --> %s\n%s\n\n", i+1, t0_str, t1_str, text);
        fflush(output_file);
    }
}
