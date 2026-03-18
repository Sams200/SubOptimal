#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "defaults.h"
#include "parser.h"
#include "loader.h"
#include "transcribe.h"
#include "file_helpers.h"

int main(int argc, char *argv[]){
    arguments* arguments = parse_args(argc, argv);

    // ensure models are present
    char model_rel_path[PATH_MAX];
    char vad_rel_path[PATH_MAX];
    char model_url[512];
    snprintf(model_rel_path, sizeof(model_rel_path), "%s%s", CONFIG_DIR, arguments->model);
    snprintf(vad_rel_path, sizeof(vad_rel_path), "%s%s", CONFIG_DIR, VAD_MODEL_NAME);
    snprintf(model_url, sizeof(model_url), "%s%s", HF_BASE_URL, arguments->model);

    if(curl_if_not_present(model_rel_path, model_url) ||
       curl_if_not_present(vad_rel_path, VAD_MODEL_URL)){
        free(arguments);
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
    float* pcm_frames = load_audio(arguments->source, &frame_count);
    if(!pcm_frames){
        fprintf(stderr, "MAIN: Failed to load audio\n");
        free(arguments);
        exit(1);
    }

    // transcribe audio
    subtitle_list* list = transcribe(model_path, pcm_frames, frame_count, vad_path);
    if(!list){
        free(pcm_frames);
        free(arguments);
        exit(1);
    }
    free(pcm_frames);

    FILE* file = fopen(arguments->output, "w");
    for(int i=0;i<list->count;i++){
        fprintf(file, "%d\n%s --> %s\n%s\n\n",
                i+1, list->segments[i].t0, list->segments[i].t1, list->segments[i].text);

        free(list->segments[i].text);
    }
    free(list->segments);
    free(list);
    fclose(file);

    // cleanup
    free(arguments);

    return 0;
}
