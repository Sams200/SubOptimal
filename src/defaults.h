//
// Created by sams on 3/4/26.
//

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <stddef.h>

#define CONFIG_DIR "/.local/share/SubOptimal/"

#define HF_BASE_URL "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/"
#define HF_RAW_URL  "https://huggingface.co/ggerganov/whisper.cpp/raw/main/"

#define VAD_MODEL_NAME "ggml-silero-v6.2.0.bin"
#define VAD_MODEL_URL  "https://huggingface.co/ggml-org/whisper-vad/resolve/main/ggml-silero-v6.2.0.bin"

// model weights
#define TRANSLATE_MODEL_URL   "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/model.bin"
// sentencepiece tokenizer
#define TRANSLATE_SPM_URL     "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/sentencepiece.bpe.model"
// config
#define TRANSLATE_CONFIG_URL  "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/config.json"
#define TRANSLATE_MODEL_DIR CONFIG_DIR "nllb-200-distilled-600M-ct2/"

static const char* VALID_MODELS[] = {
    "ggml-tiny.bin",
    "ggml-tiny.en.bin",
    "ggml-base.bin",
    "ggml-base.en.bin",
    "ggml-small.bin",
    "ggml-small.en.bin",
    "ggml-medium.bin",
    "ggml-medium.en.bin",
    "ggml-large-v3.bin",
    "ggml-large-v3-turbo.bin",
    NULL
};

#endif //DEFAULTS_H
