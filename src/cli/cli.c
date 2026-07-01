#include <stdio.h>

#include "cli.h"
#include "pipeline/context_check.h"
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
        context_check_init(args->ollama_host);

        if (translated_list) {
            context_check_subtitles(original_list, translated_list, args->ollama_model);
        } else {
            context_check_subtitles(original_list, NULL, args->ollama_model);
        }

        context_check_free();
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