#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "parser.h"
#include "subtitles.h"

int cli_run(const arguments *args);
subtitle_list* perform_transcribe(const char *model, const char *source,
                                  const char *language, int *error);
void perform_translate(subtitle_list *list, const char *target_nllb, int *error);
int write_subtitles_to_file(const char *output_path, const subtitle_list *list);

#ifdef __cplusplus
}
#endif

#endif // CLI_H