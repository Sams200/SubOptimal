//
// Created by sams on 3/4/26.
//

#include "transcribe.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "defaults.h"
#include "file_helpers.h"
#include <whisper.h>

#define MAX_CHUNK_S 20.0f
static int64_t g_chunk_offset_cs = 0;

static int download_with_curl(const char *url, const char *dest_path){
    pid_t pid = fork();

    if(pid < 0){
        perror("download_with_curl: fork");
        return -1;
    }

    if(pid == 0){
        // curl here
        execlp("curl", "curl",
               "-L",           // follow redirects
               "-f",           // fail on HTTP error status
               "-#",           // progress bar
               "-o", dest_path,
               url,
               (char *)NULL);

        // should not reach here
        perror("download_with_curl: execlp curl");
        _exit(127);
    }

    // parent
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("download_with_curl: waitpid");
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "curl exited with status %d\n", WEXITSTATUS(status));
        return -1;
    }

    return 0;
}

int ensure_model(const char *model_name, char *model_path, size_t path_size, char *vad_path, size_t vad_size){
    // $HOME/.local/share/SubOptimal/<model_name>

    // get config dir
    const char *home = getenv("HOME");
    if(!home){
        fprintf(stderr, "ensure_model: $HOME is not set\n");
        return -1;
    }
    
    // download model
    int written = snprintf(model_path, path_size,
                           "%s" CONFIG_DIR "%s", home, model_name);
    if(written < 0 || (size_t)written>=path_size){
        fprintf(stderr, "ensure_model: model path too long\n");
        return -1;
    }

    if(!file_exists(model_path)){
        mkdirs_for_file(model_path);

        char url[512];
        written = snprintf(url, sizeof(url), "%s%s", HF_BASE_URL, model_name);
        if(written<0 || (size_t)written>=sizeof(url)){
            fprintf(stderr, "ensure_model: URL too long\n");
            return -1;
        }

        fprintf(stdout, "Model '%s' not found. Downloading from:\n  %s\n",
                model_name, url);
        fflush(stdout);

        if(download_with_curl(url, model_path) != 0){
            remove(model_path);
            fprintf(stderr, "ensure_model: failed to download '%s'\n", model_name);
            return -1;
        }

        fprintf(stdout, "\nModel saved to: %s\n", model_path);
    }

    // download vad
    written = snprintf(vad_path, vad_size,
                       "%s" CONFIG_DIR "%s", home, VAD_MODEL_NAME);
    if(written < 0 || (size_t)written >= vad_size){
        fprintf(stderr, "ensure_model: VAD path too long\n");
        return -1;
    }

    if(!file_exists(vad_path)){
        mkdirs_for_file(vad_path);

        fprintf(stdout, "VAD model not found. Downloading from:\n  %s\n", VAD_MODEL_URL);
        fflush(stdout);

        if(download_with_curl(VAD_MODEL_URL, vad_path) != 0){
            remove(vad_path);
            fprintf(stderr, "ensure_model: failed to download VAD model\n");
            return -1;
        }

        fprintf(stdout, "\nVAD model saved to: %s\n", vad_path);
    }

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

static int g_segment_counter = 0;

static void whisper_log_cb(struct whisper_context *ctx,
                           struct whisper_state *state,
                           int n_new, void* user_data){
    FILE *output_file = (FILE *)user_data;
    const int n_segments = whisper_full_n_segments(ctx);
    const int i = n_segments - 1;

    if(i >= 0){
        const char *text = whisper_full_get_segment_text(ctx, i);
        int64_t t0 = whisper_full_get_segment_t0(ctx, i) + g_chunk_offset_cs;
        int64_t t1 = whisper_full_get_segment_t1(ctx, i) + g_chunk_offset_cs;

        g_segment_counter++;

        char t0_str[32], t1_str[32];
        format_srt_timestamp(t0, t0_str);
        format_srt_timestamp(t1, t1_str);

        fprintf(stdout, "[%s --> %s] %s\n", t0_str, t1_str, text);
        fprintf(output_file, "%d\n%s --> %s\n%s\n\n",
                g_segment_counter, t0_str, t1_str, text);
        fflush(output_file);
    }
}

static void whisper_silent_log_cb(enum ggml_log_level level,
                                  const char *text,
                                  void *user_data){
    (void)level;
    (void)text;
    (void)user_data;
}

int transcribe(const char *model_path, const float *audio_data,
               size_t audio_frames, const char *output_path, const char *vad_path){
    // load model
    whisper_log_set(whisper_silent_log_cb, NULL);
    struct whisper_context* ctx = whisper_init_from_file(model_path);
    if(ctx == NULL){
        fprintf(stderr, "transcribe: Failed to load model %s\n", model_path);
        return -1;
    }

    // open output file for writing
    FILE *output_file = fopen(output_path, "w");
    if(!output_file){
        fprintf(stderr, "transcribe: Failed to open output file %s\n", output_path);
        whisper_free(ctx);
        return -1;
    }

    // setup params
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
    wparams.new_segment_callback = whisper_log_cb;
    wparams.new_segment_callback_user_data = output_file;
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

    fclose(output_file);
    whisper_free(ctx);
    return 0;
}