#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cli/parser.h"
#include "../subtitles/subtitles.h"

int cli_run(const arguments *args);
void print_progress(int percent);

#ifdef __cplusplus
}
#endif

#endif // CLI_H