#include "validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int validate_srt_timestamp(const char *ts) {
    if (!ts || strlen(ts) != 12) return 0;
    if (ts[2] != ':' || ts[5] != ':' || ts[8] != ',') return 0;
    for (int i = 0; i < 12; i++) {
        if (i == 2 || i == 5 || i == 8) continue;
        if (!isdigit(ts[i])) return 0;
    }
    return 1;
}

int validate_srt_line(const char *line) {
    if (!line) return 0;
    while (*line && isspace(*line)) line++;

    if (!isdigit(*line)) return 0;
    while (isdigit(*line)) line++;

    if (*line != '\n') return 0;
    line++;

    char t0[13];
    if (strlen(line) < 12) return 0;
    memcpy(t0, line, 12);
    t0[12] = '\0';
    if (!validate_srt_timestamp(t0)) return 0;
    line += 12;

    if (strncmp(line, " --> ", 5) != 0) return 0;
    line += 5;

    char t1[13];
    if (strlen(line) < 12) return 0;
    memcpy(t1, line, 12);
    t1[12] = '\0';
    if (!validate_srt_timestamp(t1)) return 0;
    line += 12;

    if (strcmp(t0, t1) >= 0) return 0;

    if (*line != '\n') return 0;
    line++;

    if (*line == '\0' || *line == '\n') return 0;

    return 1;
}

int validate_srt_list(subtitle_list *list) {
    if (!list || (!list->segments && list->count > 0)) return 0;

    for (size_t i = 0; i < list->count; i++) {
        subtitle_segment *seg = &list->segments[i];
        if (!seg->text || seg->text[0] == '\0') return 0;
        if (!validate_srt_timestamp(seg->t0)) return 0;
        if (!validate_srt_timestamp(seg->t1)) return 0;
        if (strcmp(seg->t0, seg->t1) >= 0) return 0;
    }

    return 1;
}

subtitle_list *parse_srt_response(const char *srt) {
    if (!srt) return NULL;

    subtitle_list *list = calloc(1, sizeof(subtitle_list));
    if (!list) return NULL;

    list->capacity = 16;
    list->segments = calloc(list->capacity, sizeof(subtitle_segment));
    if (!list->segments) {
        free(list);
        return NULL;
    }

    const char *p = srt;

    while (*p) {
        // ws
        while (*p && (*p == '\n' || *p == '\r' || *p == ' ')) p++;
        if (!*p) break;

        // id
        int id = 0;
        if (!isdigit((unsigned char)*p)) { p++; continue; }
        while (isdigit((unsigned char)*p)) {
            id = id * 10 + (*p - '0');
            p++;
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;

        // timestamps
        if (strlen(p) < 29) break;

        char t0[32] = {0}, t1[32] = {0};
        memcpy(t0, p, 12); t0[12] = '\0';
        p += 12;

        if (strncmp(p, " --> ", 5) != 0) continue;
        p += 5;

        memcpy(t1, p, 12); t1[12] = '\0';
        p += 12;

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;

        // text
        const char *text_start = p;
        while (*p && !(*p == '\n' && (*(p + 1) == '\n' || *(p + 1) == '\r' || *(p + 1) == '\0'))) {
            p++;
        }
        const char *text_end = p;
        if (*p == '\n') p++;

        //ws
        while (text_end > text_start &&
               (*(text_end - 1) == '\n' || *(text_end - 1) == '\r' || *(text_end - 1) == ' ')) {
            text_end--;
        }

        size_t text_len = text_end - text_start;
        if (text_len == 0) continue;
        int all_space = 1;
        for (size_t k = 0; k < text_len; k++) {
            if (!isspace((unsigned char)text_start[k])) {
                all_space = 0;
                break;
            }
        }
        if (all_space) continue;

        if (!validate_srt_timestamp(t0) || !validate_srt_timestamp(t1)) continue;

        if (list->count >= list->capacity) {
            list->capacity *= 2;
            subtitle_segment *tmp = realloc(list->segments, list->capacity * sizeof(subtitle_segment));
            if (!tmp) { free_subtitle_list(list); return NULL; }
            list->segments = tmp;
        }

        subtitle_segment *seg = &list->segments[list->count];
        seg->id = id;
        memcpy(seg->t0, t0, 32);
        memcpy(seg->t1, t1, 32);
        seg->text = malloc(text_len + 1);
        if (!seg->text) { free_subtitle_list(list); return NULL; }
        memcpy(seg->text, text_start, text_len);
        seg->text[text_len] = '\0';
        // printf("%s\n", seg->text);
        list->count++;
    }

    if (list->count == 0) {
        free_subtitle_list(list);
        return NULL;
    }

    return list;
}