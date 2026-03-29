#include "translate.h"
#include <ctranslate2/translator.h>
#include <sentencepiece_processor.h>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdio>
#include <sys/stat.h>

static ctranslate2::Translator *translator = nullptr;
static sentencepiece::SentencePieceProcessor *spp = nullptr;

void translator_init(const char *model_path, const char *source_lang, const char *target_lang) {
    // model_path should be the directory containing the model files
    // Helsinki models: model.bin, source.spm, target.spm
    // NLLB models: model.bin, sentencepiece.bpe.model

    // Detect Helsinki model by checking for source.spm
    bool is_helsinki = false;
    char source_spm_path[4096];
    snprintf(source_spm_path, sizeof(source_spm_path), "%s/source.spm", model_path);

    struct stat st;
    if (stat(source_spm_path, &st) == 0) {
        is_helsinki = true;
    }

    translator = new ctranslate2::Translator(
        model_path,
        ctranslate2::Device::CUDA,
        ctranslate2::ComputeType::DEFAULT,
        {0}
    );

    std::string spm_path;
    if (is_helsinki) {
        spm_path = std::string(model_path) + "/target.spm";
    } else {
        spm_path = std::string(model_path) + "/sentencepiece.bpe.model";
    }

    spp = new sentencepiece::SentencePieceProcessor();
    if (!spp->Load(spm_path).ok())
        throw std::runtime_error("Failed to load SentencePiece model");
}

char *translator_translate(const char *text, const char *source, const char *target) {
    if (!translator || !spp) return nullptr;

    // tokenize
    std::vector<std::string> tokens;
    spp->Encode(text, &tokens);

    // NLLB expects the target language token at the start
    // and source language token appended at the end
    tokens.push_back("</s>");
    tokens.push_back(source);  // e.g. "eng_Latn"

    ctranslate2::TranslationOptions opts;
    opts.beam_size = 4;

    std::vector<std::string> target_prefix = {target};  // e.g. "ron_Latn"

    auto results = translator->translate_batch(
        {tokens},
        {target_prefix},
        opts
    );

    const auto &output_tokens = results[0].output();

    // detokenize, skipping the language token at index 0
    std::vector<std::string> decoded(output_tokens.begin() + 1, output_tokens.end());
    std::string out;
    spp->Decode(decoded, &out);

    return strdup(out.c_str());
}

void translator_free(void) {
    delete translator;
    delete spp;
    translator = nullptr;
    spp = nullptr;
}

int translator_is_ready(void) {
    return (translator != nullptr && spp != nullptr) ? 1 : 0;
}