//
// Created by sams on 3/4/26.
//

#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H
#include <stddef.h>
#include "parser.h"

/*
 * Ensure the whisper model and VAD model are present.
 * Downloads from HuggingFace if missing.
 *
 * model_name  - filename only, e.g. "ggml-base.en.bin"
 * model_path  - output buffer that receives the full path to the model file
 * path_size   - size of model_path buffer
 * vad_path    - output buffer for VAD model path (silero_v5.onnx)
 *
 * Returns 0 on success, -1 on failure.
 */
int ensure_model(const char *model_name, char *model_path, size_t path_size, char *vad_path, size_t vad_size);

/*
 * Transcribe audio data and write SRT output to file.
 *
 * model_path   - path to whisper.cpp model file
 * audio_data   - PCM audio data as float array (mono, 32-bit float)
 * audio_frames - number of PCM frames in audio_data
 * output_path  - path to output SRT file
 *
 * Returns 0 on success, -1 on failure.
 */
int transcribe(const char *model_path, const float *audio_data,
               size_t audio_frames, const char *output_path, const char *vad_path);
#endif //TRANSCRIBE_H
