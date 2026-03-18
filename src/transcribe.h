//
// Created by sams on 3/4/26.
//

#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H
#include <stddef.h>

typedef struct {
    char t0[32];
    char t1[32];
    char *text;
} subtitle_segment;

typedef struct {
    subtitle_segment *segments;
    size_t count;
    size_t capacity;
} subtitle_list;

/*
 * Transcribe audio data and return list of subtitle segments.
 *
 * model_path   - path to whisper.cpp model file
 * audio_data   - PCM audio data as float array (mono, 32-bit float)
 * audio_frames - number of PCM frames in audio_data
 * vad_path     - path to VAD model
 *
 * Returns allocated subtitle_list* (caller must free), or NULL on failure.
 */
subtitle_list* transcribe(const char *model_path, const float *audio_data,
               size_t audio_frames, const char *vad_path);
#endif //TRANSCRIBE_H
