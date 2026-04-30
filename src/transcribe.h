//
// Created by sams on 3/4/26.
//

#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H
#include <stddef.h>
#include "subtitles.h"

/*
 * Transcribe audio data and return list of subtitle segments.
 *
 * model_path   - path to whisper.cpp model file
 * audio_data   - PCM audio data as float array (mono, 32-bit float)
 * audio_frames - number of PCM frames in audio_data
 * vad_path     - path to VAD model
 * language     - input audio language code (e.g., "ja", "en"). If NULL, auto-detect
 *
 * Returns allocated subtitle_list* (caller must free), or NULL on failure.
 */
subtitle_list* transcribe(const char *model_path, const float *audio_data,
               size_t audio_frames, const char *vad_path, const char *language);

int get_transcribe_progress_percent();
#endif //TRANSCRIBE_H
