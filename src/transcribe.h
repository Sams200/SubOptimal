//
// Created by sams on 3/4/26.
//

#ifndef TRANSCRIBE_H
#define TRANSCRIBE_H
#include <stddef.h>

/*
 * Ensure the given whisper model is present
 * If the file is missing, it is downloaded via curl from HuggingFace.
 *
 * model_name  - filename only, e.g. "ggml-base.en.bin"
 * model_path  - output buffer that receives the full path to the model file
 * path_size   - size of model_path buffer
 *
 * Returns 0 on success, -1 on failure.
 */
int ensure_model(const char *model_name, char *model_path, size_t path_size);

#endif //TRANSCRIBE_H
