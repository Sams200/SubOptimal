#include "translate.h"
#include <ctranslate2/translator.h>
#include <sentencepiece_processor.h>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdio>

#include "defaults.h"
#include "cli_ui.h"

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

void translate_subtitles(subtitle_list* subtitles, const char *source, const char *target) {
    if (!subtitles || !subtitles->segments || subtitles->count == 0) return;

    printf("Translating subtitles:\n");

    ctranslate2::TranslationOptions opts;
    opts.beam_size = 2;

    const size_t batch_size = 24;

    // Encode all text to tokens upfront
    std::vector<std::vector<std::string>> all_tokens;
    std::vector<std::vector<std::string>> target_prefixes;

    for (size_t i = 0; i < subtitles->count; i++) {
        std::string text = subtitles->segments[i].text ? subtitles->segments[i].text : "";
        std::vector<std::string> tokens;
        spp->Encode(text, &tokens);
        tokens.push_back("</s>");
        tokens.push_back(source);
        all_tokens.push_back(tokens);
        target_prefixes.push_back({target});
    }

    // Translate in batches
    for (size_t offset = 0; offset < subtitles->count; offset += batch_size) {
        // Display progress bar
        int progress = offset * 100 / subtitles->count;
        print_progress(progress);

        size_t end = offset + batch_size;
        if (end > subtitles->count)
            end = subtitles->count;

        std::vector<std::vector<std::string>> batch_tokens(
            all_tokens.begin() + offset, all_tokens.begin() + end);
        std::vector<std::vector<std::string>> batch_prefixes(
            target_prefixes.begin() + offset, target_prefixes.begin() + end);

        auto results = translator->translate_batch(batch_tokens, batch_prefixes, opts);

        // Decode tokens to text
        for (size_t j = 0; j < results.size(); j++) {
            size_t idx = offset + j;
            if (results[j].output().empty())
                continue;

            const auto &output_tokens = results[j].output();
            std::vector<std::string> decoded(output_tokens.begin() + 1, output_tokens.end());
            std::string out;
            spp->Decode(decoded, &out);

            if (subtitles->segments[idx].text)
                free(subtitles->segments[idx].text);
            subtitles->segments[idx].text = strdup(out.c_str());
        }
    }

    int lang_idx = 0;
    while(!is_end_of_array(VALID_LANGUAGES[lang_idx])){
        if(strncmp_safe(VALID_LANGUAGES[lang_idx], target, 10)){
            subtitles->language = LANGUAGE_NAMES[lang_idx];
            break;
        }
        lang_idx++;
    }

    print_progress(100);
    printf("\n");
}

void translator_free(void) {
    delete translator;
    delete spp;
    translator = nullptr;
    spp = nullptr;
}
