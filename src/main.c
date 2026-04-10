#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "defaults.h"
#include "parser.h"
#include "loader.h"
#include "subtitles.h"
#include "transcribe.h"
#include "file_helpers.h"
#include "post_processing.h"
#include "translate.h"
#include "context_check.h"

void write_subtitles_to_file(const char* output_path,const subtitle_list* list){
    FILE* file = fopen(output_path, "w");
    for(int i=0;i<list->count;i++){
        // skip deleted segments
        if(list->segments[i].text[0] == '*')
            continue;
        fprintf(file, "%d\n%s --> %s\n%s\n\n",
                i+1, list->segments[i].t0, list->segments[i].t1, list->segments[i].text);
    }
    fclose(file);
}

subtitle_list* perform_transcribe(const char* model, const char* source, const char* language){
    // TRANSCRIBE /////////////////////////
    // ensure models are present
    char model_rel_path[PATH_MAX];
    char vad_rel_path[PATH_MAX];
    char model_url[512];
    snprintf(model_rel_path, sizeof(model_rel_path), "%s%s", CONFIG_DIR, model);
    snprintf(vad_rel_path, sizeof(vad_rel_path), "%s%s", CONFIG_DIR, VAD_MODEL_NAME);
    snprintf(model_url, sizeof(model_url), "%s%s", HF_BASE_URL, model);

    if(curl_if_not_present(model_rel_path, model_url) ||
       curl_if_not_present(vad_rel_path, VAD_MODEL_URL)){
        exit(1);
    }

    // build full paths for transcribe
    char model_path[PATH_MAX];
    char vad_path[PATH_MAX];
    const char *home = getenv("HOME");
    snprintf(model_path, sizeof(model_path), "%s%s", home, model_rel_path);
    snprintf(vad_path, sizeof(vad_path), "%s%s", home, vad_rel_path);

    // load audio
    size_t frame_count;
    float* pcm_frames = load_audio(source, &frame_count);
    if(!pcm_frames){
        fprintf(stderr, "MAIN: Failed to load audio\n");
        exit(1);
    }

    // transcribe audio
    subtitle_list* list = transcribe(model_path, pcm_frames, frame_count, vad_path, language);
    if(!list){
        free(pcm_frames);
        exit(1);
    }
    free(pcm_frames);

    return list;
}

void perform_translate(subtitle_list *list, const char *target_nllb) {
    // Map whisper language code to NLLB code
    const char *source_nllb = NULL;

    if (list->language) {
        int source_idx = is_valid_option(list->language, WHISPER_LANGUAGE_CODES);
        if (source_idx >= 0 && VALID_LANGUAGES[source_idx] != NULL) {
            source_nllb = VALID_LANGUAGES[source_idx];
        }
    }

    // Check that source and target are different
    if (source_nllb && strcmp(source_nllb, target_nllb) == 0) {
        printf("Source and target languages are the same, skipping translation\n");
        return;
    }

    char translate_model_rel_path[PATH_MAX];
    char spm_rel_path[PATH_MAX];
    char config_rel_path[PATH_MAX];
    char vocab_rel_path[PATH_MAX];
    char tokenizer_rel_path[PATH_MAX];
    char tokenizer_cfg_rel_path[PATH_MAX];
    char special_tokens_rel_path[PATH_MAX];

    snprintf(translate_model_rel_path, sizeof(translate_model_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_MODEL_NAME);
    snprintf(spm_rel_path, sizeof(spm_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_SPM_NAME);
    snprintf(config_rel_path, sizeof(config_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_CONFIG_NAME);
    snprintf(vocab_rel_path, sizeof(vocab_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_VOCAB_NAME);
    snprintf(tokenizer_rel_path, sizeof(tokenizer_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_TOKENIZER_NAME);
    snprintf(tokenizer_cfg_rel_path, sizeof(tokenizer_cfg_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_TOKENIZER_CFG_NAME);
    snprintf(special_tokens_rel_path, sizeof(special_tokens_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_SPECIAL_TOKENS_NAME);

    curl_if_not_present(translate_model_rel_path, NLLB_MODEL_URL);
    curl_if_not_present(spm_rel_path, NLLB_SPM_URL);
    curl_if_not_present(config_rel_path, NLLB_CONFIG_URL);
    curl_if_not_present(vocab_rel_path, NLLB_VOCAB_URL);
    curl_if_not_present(tokenizer_rel_path, NLLB_TOKENIZER_URL);
    curl_if_not_present(tokenizer_cfg_rel_path, NLLB_TOKENIZER_CFG_URL);
    curl_if_not_present(special_tokens_rel_path, NLLB_SPECIAL_TOKENS_URL);

    char translate_model_path[PATH_MAX];
    char spm_path[PATH_MAX];
    const char *home = getenv("HOME");
    snprintf(translate_model_path, sizeof(translate_model_path), "%s%s", home, NLLB_MODEL_DIR);
    snprintf(spm_path, sizeof(spm_path), "%s%s", home, spm_rel_path);

    translator_init(translate_model_path, spm_path);
    if(source_nllb){
        translate_subtitles(list, source_nllb, target_nllb);
    }
    translator_free();
}

int main(int argc, char *argv[]){
    arguments* arguments = parse_args(argc, argv);

    subtitle_list *original_list = perform_transcribe(arguments->model, arguments->source, arguments->language);
    subtitle_list* translated_list = NULL;

    if (arguments->translate) {
        merged_list *merged = merge_sentences(original_list);

        perform_translate(merged->subs, arguments->translate);

        translated_list = split_for_display(merged, 80);
        free_merged_list(merged);
    }

    if (arguments->ollama_model) {
        context_check_init(arguments->ollama_host);

        if (translated_list) {
            context_check_subtitles(original_list, translated_list, arguments->ollama_model);
        }
        else {
            context_check_subtitles(original_list, NULL, arguments->ollama_model);
        }
        
        context_check_free();
    }

    if (translated_list) {
        write_subtitles_to_file(arguments->output, translated_list);
        free_subtitle_list(translated_list);
    }
    else {
        write_subtitles_to_file(arguments->output, original_list);
    }

    free_subtitle_list(original_list);
    free(arguments);
    return 0;
}
