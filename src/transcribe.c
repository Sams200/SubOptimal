//
// Created by sams on 3/4/26.
//

#include "transcribe.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "defaults.h"
#include <whisper.h>

#define MAX_CHUNK_S 20.0f
static int64_t g_chunk_offset_cs = 0;

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

static void whisper_log_cb(struct whisper_context *ctx,
                           struct whisper_state *state,
                           int n_new, void* user_data){
    subtitle_list *list = (subtitle_list *)user_data;
    const int n_segments = whisper_full_n_segments(ctx);
    const int i = n_segments - 1;

    if (i >= 0) {
        // grow array if needed
        if (list->count >= list->capacity) {
            list->capacity = list->capacity ? list->capacity * 2 : 64;
            list->segments = realloc(list->segments,
                                     list->capacity * sizeof(subtitle_segment));
        }

        subtitle_segment *seg = &list->segments[list->count++];
        int64_t t0 = whisper_full_get_segment_t0(ctx, i) + g_chunk_offset_cs;
        int64_t t1 = whisper_full_get_segment_t1(ctx, i) + g_chunk_offset_cs;

        format_srt_timestamp(t0, seg->t0);
        format_srt_timestamp(t1, seg->t1);
        seg->text = strdup(whisper_full_get_segment_text(ctx, i));

        fprintf(stdout, "[%s --> %s] %s\n", seg->t0, seg->t1, seg->text);

    }
}

static void whisper_silent_log_cb(enum ggml_log_level level,
                                  const char *text,
                                  void *user_data){
    (void)level;
    (void)text;
    (void)user_data;
}

subtitle_list* transcribe(const char *model_path, const float *audio_data,
               size_t audio_frames, const char *vad_path){
    g_chunk_offset_cs = 0;

    // load model
    whisper_log_set(whisper_silent_log_cb, NULL);
    struct whisper_context* ctx = whisper_init_from_file(model_path);
    if(ctx == NULL){
        fprintf(stderr, "transcribe: Failed to load model %s\n", model_path);
        return NULL;
    }

    subtitle_list *list = malloc(sizeof(subtitle_list));
    if(list == NULL){
        whisper_free(ctx);
        return NULL;
    }

    // initialize list
    list->segments = NULL;
    list->count = 0;
    list->capacity = 0;

    // setup params
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
    wparams.new_segment_callback = whisper_log_cb;
    wparams.new_segment_callback_user_data = list;
    wparams.n_threads = 8;

    // 5 beams running in parallel
    wparams.beam_search.beam_size = 5;

    // prevent repetition loops
    wparams.entropy_thold = 2.8f;
    wparams.suppress_blank = 1;
    wparams.suppress_nst = 1;

    // segment splitting - enforce stricter limits
    wparams.single_segment = 0;      // allow multiple segments
    wparams.split_on_word = 1;       // split on word boundaries
    wparams.max_len = 50;            // max 50 chars per segment
    wparams.max_tokens = 0;          // no token limit (use text length instead)

    // force more frequent segment breaks
    wparams.no_context = 0;          // don't use past transcription as context
    wparams.n_max_text_ctx = 32;     // limit context window

    // voice activity detection
    wparams.vad = 1;
    wparams.vad_model_path = vad_path;
    wparams.vad_params.threshold = 0.40f;             // vad threshold
    wparams.vad_params.min_speech_duration_ms = 100;  // min 100ms speech
    wparams.vad_params.min_silence_duration_ms = 200; // 200ms silence ends segment
    wparams.vad_params.max_speech_duration_s = 5.0f;  // max 5s per segment
    wparams.vad_params.speech_pad_ms = 300;           // 300ms padding around segments

    // perform inference
    printf("BEGIN TRANSCRIPT\n");

    int n_samples_chunk = (int)(MAX_CHUNK_S * WHISPER_SAMPLE_RATE);

    for (size_t offset = 0; offset < audio_frames; offset += n_samples_chunk) {
        size_t chunk_len = (offset + n_samples_chunk > audio_frames)
                        ? (audio_frames - offset)
                        : n_samples_chunk;

        whisper_full(ctx, wparams, audio_data + offset, (int)chunk_len);

        // advance offset by how many samples got processed
        g_chunk_offset_cs = (int64_t)((offset + chunk_len) / (float)WHISPER_SAMPLE_RATE * 100);
    }

    whisper_free(ctx);
    return list;
}