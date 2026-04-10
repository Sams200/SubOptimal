//
// Created by sams on 4/8/26.
//

#ifndef SUBTITLES_H
#define SUBTITLES_H
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char t0[32];
    char t1[32];
    char *text;
} subtitle_segment;

typedef struct {
    subtitle_segment *segments;
    size_t count;
    size_t capacity;
    const char *language;
} subtitle_list;

subtitle_list* duplicate_list(subtitle_list* list);
void free_subtitle_list(subtitle_list *list);

#endif //SUBTITLES_H
