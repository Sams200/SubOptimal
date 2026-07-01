#include <stdio.h>

#include "cli.h"
#include "pipeline/pipeline.h"




int cli_run(const arguments *args) {
    int error;

    subtitle_list *original_list = perform_transcribe(args->model, args->source, args->language, &error);
    if (error) return error;

    subtitle_list *translated_list = NULL;

    if (args->translate) {
        translated_list = duplicate_list(original_list);
        perform_translate(translated_list, args->translate, &error);
        if (error) {
            free_subtitle_list(translated_list);
            free_subtitle_list(original_list);
            return error;
        }
    }

    if (args->ollama_model) {
        context_check(args->ollama_host, args->ollama_model,original_list,translated_list);
    }

    if (translated_list) {
        error = write_subtitles_to_file(args->output, translated_list);
        free_subtitle_list(translated_list);
    } else {
        error = write_subtitles_to_file(args->output, original_list);
    }

    free_subtitle_list(original_list);
    return error;
}

void print_progress(int percent) {
    int bar_width = 50;
    int filled = bar_width * percent / 100;

    printf("\r[");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) printf("#");
        else printf(" ");
    }
    printf("] %d%%", percent);
    fflush(stdout);
}