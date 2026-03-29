#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "defaults.h"
#include "parser.h"
#include "loader.h"
#include "transcribe.h"
#include "file_helpers.h"
#include "translate.h"

void write_subtitles_to_file(const char* output_path,const subtitle_list* list){
    FILE* file = fopen(output_path, "w");
    for(int i=0;i<list->count;i++){
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

int main(int argc, char *argv[]){
    arguments* arguments = parse_args(argc, argv);

    subtitle_list* list = perform_transcribe(arguments->model, arguments->source, arguments->language);

    if(arguments->translate){
        // TRANSLATE /////////////////////////

        // Map whisper language code to NLLB code
        const char *source_nllb = NULL;
        const char *target_nllb = NULL;

        if(list->language){
            int source_idx = is_valid_option(list->language, WHISPER_LANGUAGE_CODES);
            if(source_idx >= 0 && VALID_LANGUAGES[source_idx] != NULL){
                source_nllb = VALID_LANGUAGES[source_idx];
            }
        }

        target_nllb = arguments->translate;  // already in NLLB format

        // Check that source and target are different
        if(source_nllb && strcmp(source_nllb, target_nllb) == 0){
            printf("Source and target languages are the same, skipping translation\n");
        }
        else if(!source_nllb){
            // Language not detected, fall back to NLLB
            goto LABEL_NLLB;
        }
        else{
            char translate_model_rel_path[PATH_MAX];
            char spm_rel_path[PATH_MAX];
            char config_rel_path[PATH_MAX];
            const char *model_url = NULL;
            const char *spm_url = NULL;
            const char *config_url = NULL;
            const char *model_dir = NULL;

            // Check if source or target is English -> use Helsinki
            int source_is_en = (source_nllb && strncmp(source_nllb, "eng", 3) == 0);
            int target_is_en = (strncmp(target_nllb, "eng", 3) == 0);

            if(source_is_en || target_is_en){
                // Use Helsinki model - find language index
                const char *helsinki_dir;
                const char *helsinki_url;

                if(source_is_en){
                    // English -> other language
                    int lang_idx = is_valid_option(target_nllb, VALID_LANGUAGES);
                    if(lang_idx < 0){
                        fprintf(stderr, "Unknown target language: %s\n", target_nllb);
                        exit(1);
                    }
                    helsinki_dir = HELSINKI_EN_TO_LANG_DIR[lang_idx];
                    helsinki_url = HELSINKI_EN_TO_LANG_URL[lang_idx];
                }
                else{
                    // Other language -> English
                    int lang_idx = is_valid_option(source_nllb, VALID_LANGUAGES);
                    if(lang_idx < 0){
                        fprintf(stderr, "Helsinki model not available for language: %s, falling back to NLLB\n", source_nllb);
                        goto LABEL_NLLB;
                    }
                    helsinki_dir = HELSINKI_LANG_TO_EN_DIR[lang_idx];
                    helsinki_url = HELSINKI_LANG_TO_EN_URL[lang_idx];
                }

                if(!helsinki_dir || !helsinki_url){
                    goto LABEL_NLLB;
                }

                // Build Helsinki model paths
                snprintf(translate_model_rel_path, sizeof(translate_model_rel_path), "%s%s", CONFIG_DIR, helsinki_dir);

                // Download converted Helsinki models
                // source.spm, target.spm, model.bin
                char src_spm_url[512], tgt_spm_url[512], model_bin_url[512];
                char src_spm_path[PATH_MAX], tgt_spm_path[PATH_MAX], model_bin_path[PATH_MAX];

                snprintf(src_spm_url, sizeof(src_spm_url), "%s/source.spm", helsinki_url);
                snprintf(tgt_spm_url, sizeof(tgt_spm_url), "%s/target.spm", helsinki_url);
                snprintf(model_bin_url, sizeof(model_bin_url), "%s/model.bin", helsinki_url);

                snprintf(src_spm_path, sizeof(src_spm_path), "%s%s/source.spm", CONFIG_DIR, helsinki_dir);
                snprintf(tgt_spm_path, sizeof(tgt_spm_path), "%s%s/target.spm", CONFIG_DIR, helsinki_dir);
                snprintf(model_bin_path, sizeof(model_bin_path), "%s%s/model.bin", CONFIG_DIR, helsinki_dir);

                curl_if_not_present(src_spm_path, src_spm_url);
                curl_if_not_present(tgt_spm_path, tgt_spm_url);
                curl_if_not_present(model_bin_path, model_bin_url);

                printf("Helsinki model selected: %s\n", helsinki_dir);
            }
            else{
                LABEL_NLLB:
                // Use NLLB model (neither source nor target is English, or no Helsinki is available)
                snprintf(translate_model_rel_path, sizeof(translate_model_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_MODEL_NAME);
                snprintf(spm_rel_path, sizeof(spm_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_SPM_NAME);
                snprintf(config_rel_path, sizeof(config_rel_path), "%s%s", NLLB_MODEL_DIR, NLLB_CONFIG_NAME);

                // download translation model files if needed
                curl_if_not_present(translate_model_rel_path, NLLB_MODEL_URL);
                curl_if_not_present(spm_rel_path, NLLB_SPM_URL);
                curl_if_not_present(config_rel_path, NLLB_CONFIG_URL);
            }

            // build full paths for translate
            char translate_model_path[PATH_MAX];
            char spm_path[PATH_MAX];
            char config_path[PATH_MAX];
            const char *home = getenv("HOME");
            snprintf(translate_model_path, sizeof(translate_model_path), "%s%s", home, translate_model_rel_path);
            snprintf(spm_path, sizeof(spm_path), "%s%s", home, spm_rel_path);
            snprintf(config_path, sizeof(config_path), "%s%s", home, config_rel_path);

            // TODO run translate
        }
    }

    write_subtitles_to_file(arguments->output,list);

    // cleanup
    for(int i=0;i<list->count;i++){
        free(list->segments[i].text);
    }
    free(list->segments);
    free(list);
    free(arguments);
    return 0;
}
