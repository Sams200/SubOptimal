#include "translate.h"
#include <ctranslate2/translator.h>
#include <sentencepiece_processor.h>
#include <string>
#include <cstring>
#include <stdexcept>

static ctranslate2::Translator *translator = nullptr;
static sentencepiece::SentencePieceProcessor *spp = nullptr;

void translator_init(const char *model_path) {
    // model_path should be the directory containing
    // model.bin, tokenizer.model, etc.
    translator = new ctranslate2::Translator(
        model_path,
        ctranslate2::Device::CUDA,
        ctranslate2::ComputeType::DEFAULT,
        {0}  // device indices
    );

    // tokenizer.model ships alongside the NLLB model
    std::string spm_path = std::string(model_path) + "sentencepiece.bpe.model";
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