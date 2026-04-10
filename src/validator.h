#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "subtitles.h"

int validate_srt_timestamp(const char *ts);
int validate_srt_line(const char *line);
int validate_srt_list(subtitle_list *list);

subtitle_list *parse_srt_response(const char *srt);

#endif