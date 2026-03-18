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
    int result = transcribe(model_path, pcm_frames, frame_count, arguments->output, vad_path);
    if(result != 0){
        exit(1);
    }

    // cleanup
    free(pcm_frames);
    free(arguments);

    return 0;
}
