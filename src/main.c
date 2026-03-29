#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "defaults.h"
#include "parser.h"
#include "loader.h"
#include "transcribe.h"
#include "file_helpers.h"

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
        // download translation model files if needed
        char translate_model_rel_path[PATH_MAX];
        char spm_rel_path[PATH_MAX];
        char config_rel_path[PATH_MAX];
        snprintf(translate_model_rel_path, sizeof(translate_model_rel_path), "%s%s", TRANSLATE_MODEL_DIR, TRANSLATE_MODEL_NAME);
        snprintf(spm_rel_path, sizeof(spm_rel_path), "%s%s", TRANSLATE_MODEL_DIR, TRANSLATE_SPM_NAME);
        snprintf(config_rel_path, sizeof(config_rel_path), "%s%s", TRANSLATE_MODEL_DIR, TRANSLATE_CONFIG_NAME);

        curl_if_not_present(translate_model_rel_path, TRANSLATE_MODEL_URL);
        curl_if_not_present(spm_rel_path, TRANSLATE_SPM_URL);
        curl_if_not_present(config_rel_path, TRANSLATE_CONFIG_URL);

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
