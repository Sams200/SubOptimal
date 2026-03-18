#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "parser.h"
#include "loader.h"
#include "transcribe.h"

int main(int argc, char *argv[]){
    arguments* arguments = parse_args(argc, argv);

    char model_path[PATH_MAX];
    char vad_path[PATH_MAX];
    if(ensure_model(arguments->model, model_path, PATH_MAX, vad_path, PATH_MAX)){
        exit(1);
    }

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
