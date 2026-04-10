//
// Created by sams on 4/8/26.
//

#ifndef SUBTITLES_H
#define SUBTITLES_H
#include <stdlib.h>

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

static inline void free_subtitle_list(subtitle_list *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->segments[i].text);
    }
    free(list->segments);
    free(list);
}

typedef struct {
    char t0[32];
    char t1[32];
    char *text;

    // Original fragment timestamps for
    size_t n_breaks;
    int64_t *break_times;  // centisecond timestamps at fragment boundaries
    size_t *break_chars;   // character offset in source text at each boundary
    size_t source_len;     // total character length of source (pre-translation) text
} merged_segment;

typedef struct {
    merged_segment *segments;
    size_t count;
    size_t capacity;
    char *language;
    subtitle_list *subs;   // flat subtitle_list view
} merged_list;

static inline void free_merged_list(merged_list *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->segments[i].text);
        free(list->segments[i].break_times);
        free(list->segments[i].break_chars);
    }
    free(list->segments);

    if (list->subs) {
        for (size_t i = 0; i < list->subs->count; i++) {
            free(list->subs->segments[i].text);
        }
        free(list->subs->segments);
        free(list->subs);
    }
    free(list);
}
#endif //SUBTITLES_H
