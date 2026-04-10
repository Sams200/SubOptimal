//
// Created by sams on 4/11/26.
//

#include "subtitles.h"

subtitle_list* duplicate_list(subtitle_list* list) {
    if(!list) return NULL;
    subtitle_list* new_list = (subtitle_list*)malloc(sizeof(subtitle_list));

    new_list->count = list->count;
    new_list->capacity = list->capacity;
    new_list->language = list->language;

    new_list->segments = (subtitle_segment*)malloc(sizeof(subtitle_segment) * new_list->capacity);
    for(int i=0; i<new_list->count; i++) {
        new_list->segments[i].id = list->segments[i].id;
        strcpy(new_list->segments[i].t0, list->segments[i].t0);
        strcpy(new_list->segments[i].t1, list->segments[i].t1);
        new_list->segments[i].text = strdup(list->segments[i].text);
    }

    return new_list;
}
void free_subtitle_list(subtitle_list *list) {
    if(!list) return;
    for(size_t i=0; i<list->count; i++) {
        free(list->segments[i].text);
    }
    free(list->segments);
    free(list);
}
