#include "translate.h"
#include <ctranslate2/translator.h>
#include <sentencepiece_processor.h>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdio>

static ctranslate2::Translator *translator = nullptr;
static sentencepiece::SentencePieceProcessor *spp = nullptr;

void translator_init(const char *model_path, const char *spm_path) {
    translator = new ctranslate2::Translator(
        model_path,
        ctranslate2::Device::CUDA,
        ctranslate2::ComputeType::DEFAULT,
        {0}
    );

    spp = new sentencepiece::SentencePieceProcessor();
    if (!spp->Load(spm_path).ok())
        throw std::runtime_error("Failed to load SentencePiece model");
}

char *translator_translate(const char *text, const char *source, const char *target) {
    if (!translator || !spp) return nullptr;

    // tokenize
    std::vector<std::string> tokens;
    spp->Encode(text, &tokens);

    // NLLB expects target language token at start and source at end
    tokens.push_back("</s>");
    tokens.push_back(source);

    ctranslate2::TranslationOptions opts;
    opts.beam_size = 4;

    std::vector<std::string> target_prefix = {target};

    auto results = translator->translate_batch(
        {tokens},
        {target_prefix},
        opts
    );

    const auto &output_tokens = results[0].output();

    // detokenize, skipping the target language token at index 0
    std::vector<std::string> decoded(output_tokens.begin() + 1, output_tokens.end());
    std::string out;
    spp->Decode(decoded, &out);

    return strdup(out.c_str());
}

void translate_subtitles(subtitle_list* subtitles, const char *source, const char *target) {
    if (!subtitles || !subtitles->segments || subtitles->count == 0) return;

    printf("BEGIN TRANSLATION\n");

    for (size_t i = 0; i < subtitles->count; i++) {
        subtitle_segment *seg = &subtitles->segments[i];
        if (seg->text) {
            char *translated = translator_translate(seg->text, source, target);
            if (translated) {
                free(seg->text);
                seg->text = translated;
                fprintf(stdout, "[%s --> %s] %s\n", seg->t0, seg->t1, seg->text);
            }
        }
    }
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