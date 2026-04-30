#ifndef CONTEXT_CHECK_H
#define CONTEXT_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subtitles.h"

int context_check_init(const char *ollama_host);
void context_check_subtitles(
    subtitle_list *original,
    subtitle_list *translated,
    const char *model);
void context_check_free(void);

int get_context_progress_percent();

#ifdef __cplusplus
}
#endif

#endif
