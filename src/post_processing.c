//
// Created by sams on 4/8/26.
//

#include "post_processing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int64_t parse_ts(const char *ts) {
    int h, m, s, ms;
    if (sscanf(ts, "%d:%d:%d,%d", &h, &m, &s, &ms) != 4) return 0;
    return (int64_t)h * 360000 + (int64_t)m * 6000 + (int64_t)s * 100 + ms / 10;
}

static void format_ts(int64_t cs, char *buf) {
    int h  = (int)(cs / 360000); cs %= 360000;
    int m  = (int)(cs / 6000);   cs %= 6000;
    int s  = (int)(cs / 100);
    int ms = (int)(cs % 100) * 10;
    snprintf(buf, 32, "%02d:%02d:%02d,%03d", h, m, s, ms);
}

static int is_sentence_end(char c) {
    return c == '.' || c == '!' || c == '?';
}

static void push_subtitle(subtitle_list *list, int64_t t0, int64_t t1,
                           const char *text, size_t len) {
    while (len > 0 && text[0] == ' ') { text++; len--; }

    while (len > 0 && text[len - 1] == ' ') { len--; }
    if (len == 0) return;

    if (list->count >= list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 64;
        list->segments = realloc(list->segments,
                                 list->capacity * sizeof(subtitle_segment));
    }
    subtitle_segment *seg = &list->segments[list->count++];
    format_ts(t0, seg->t0);
    format_ts(t1, seg->t1);
    seg->text = malloc(len + 1);
    memcpy(seg->text, text, len);
    seg->text[len] = '\0';
}

static void push_merged(merged_list *ml, const char *t0, const char *t1,
                         char *text, size_t source_len,
                         int64_t *break_times, size_t *break_chars,
                         size_t n_breaks) {
    if (ml->count >= ml->capacity) {
        ml->capacity = ml->capacity ? ml->capacity * 2 : 64;
        ml->segments = realloc(ml->segments,
                               ml->capacity * sizeof(merged_segment));
    }
    merged_segment *seg = &ml->segments[ml->count++];
    strncpy(seg->t0, t0, 31); seg->t0[31] = '\0';
    strncpy(seg->t1, t1, 31); seg->t1[31] = '\0';
    seg->text = text;
    seg->source_len = source_len;
    seg->n_breaks = n_breaks;

    if (n_breaks > 0) {
        seg->break_times = malloc(n_breaks * sizeof(int64_t));
        seg->break_chars = malloc(n_breaks * sizeof(size_t));
        memcpy(seg->break_times, break_times, n_breaks * sizeof(int64_t));
        memcpy(seg->break_chars, break_chars, n_breaks * sizeof(size_t));
    } else {
        seg->break_times = NULL;
        seg->break_chars = NULL;
    }
}

merged_list* merge_sentences(subtitle_list *input) {
    if (!input) return NULL;

    merged_list *ml = calloc(1, sizeof(merged_list));
    ml->language = input->language;

    char *buffer = NULL;
    size_t buf_len = 0;
    char t0_buf[32] = {0};

    // Track fragment boundaries within the current buffer
    int64_t *btimes = NULL;
    size_t *bchars = NULL;
    size_t n_breaks = 0;
    size_t breaks_cap = 0;

    for (size_t i = 0; i < input->count; i++) {
        subtitle_segment *seg = &input->segments[i];
        const char *text = seg->text;
        if (!text) continue;

        // Skip leading whitespace
        while (*text == ' ') text++;
        if (*text == '\0') continue;

        size_t add_len = strlen(text);

        if (!buffer) {
            // Start new sentence
            buffer = strdup(text);
            buf_len = add_len;
            strncpy(t0_buf, seg->t0, 31);
            n_breaks = 0;
        } else {
            // Record the boundary between previous fragment and this one
            if (n_breaks >= breaks_cap) {
                breaks_cap = breaks_cap ? breaks_cap * 2 : 16;
                btimes = realloc(btimes, breaks_cap * sizeof(int64_t));
                bchars = realloc(bchars, breaks_cap * sizeof(size_t));
            }
            btimes[n_breaks] = parse_ts(seg->t0);
            bchars[n_breaks] = buf_len + 1;
            n_breaks++;

            // Append with space
            buffer = realloc(buffer, buf_len + 1 + add_len + 1);
            buffer[buf_len] = ' ';
            memcpy(buffer + buf_len + 1, text, add_len + 1);
            buf_len = buf_len + 1 + add_len;
        }

        // Check if sentence is complete
        if (buf_len > 0 && is_sentence_end(buffer[buf_len - 1])) {
            push_merged(ml, t0_buf, seg->t1, buffer, buf_len,
                        btimes, bchars, n_breaks);
            buffer = NULL;
            buf_len = 0;
            n_breaks = 0;
        }
    }

    // Flush remainder
    if (buffer) {
        const char *last_t1 = input->segments[input->count - 1].t1;
        push_merged(ml, t0_buf, last_t1, buffer, buf_len,
                    btimes, bchars, n_breaks);
        buffer = NULL;
    }

    free(btimes);
    free(bchars);

    // Build the flat subtitle_list view for the translator
    ml->subs = calloc(1, sizeof(subtitle_list));
    ml->subs->language = ml->language;
    ml->subs->count = ml->count;
    ml->subs->capacity = ml->count;
    ml->subs->segments = malloc(ml->count * sizeof(subtitle_segment));

    for (size_t i = 0; i < ml->count; i++) {
        strncpy(ml->subs->segments[i].t0, ml->segments[i].t0, 31);
        strncpy(ml->subs->segments[i].t1, ml->segments[i].t1, 31);
        ml->subs->segments[i].text = strdup(ml->segments[i].text);
    }

    return ml;
}

static void conflict_resolve(subtitle_segment *current, subtitle_segment *previous) {
    int64_t t0 = parse_ts(previous->t0);
    int64_t t1 = parse_ts(current->t1);

    if (t1 <= t0) return;

    size_t prev_len = previous->text ? strlen(previous->text) : 0;
    size_t curr_len = current->text ? strlen(current->text) : 0;
    size_t total_len = prev_len + curr_len;

    if (total_len == 0) return;

    double frac = (double)prev_len / total_len;
    int64_t split = t0 + (int64_t)(frac * (t1 - t0));

    // ensure at least 1cs gap
    if (split <= t0) split = t0 + 1;
    if (split >= t1) split = t1 - 1;

    format_ts(split, previous->t1);
    format_ts(split, current->t0);
}

subtitle_list* split_for_display(merged_list *merged, size_t max_chars) {
    if (!merged) return NULL;

    subtitle_list *output = calloc(1, sizeof(subtitle_list));
    output->language = merged->language;

    for (size_t i = 0; i < merged->count; i++) {
        merged_segment *ms = &merged->segments[i];

        const char *text = merged->subs->segments[i].text;
        if (!text) continue;

        size_t total_len = strlen(text);
        int64_t seg_t0 = parse_ts(ms->t0);
        int64_t seg_t1 = parse_ts(ms->t1);

        // build the full list of original timestamp boundaries
        size_t n_boundaries = ms->n_breaks + 2;
        int64_t *times = malloc(n_boundaries * sizeof(int64_t));
        times[0] = seg_t0;
        for (size_t b = 0; b < ms->n_breaks; b++) {
            times[b + 1] = ms->break_times[b];
        }
        times[n_boundaries - 1] = seg_t1;

        if (total_len <= max_chars) {
            push_subtitle(output, seg_t0, seg_t1, text, total_len);
            free(times);
            continue;
        }

        size_t *splits = NULL;
        size_t n_splits = 0;
        size_t splits_cap = 0;
        size_t pos = 0;

        while (pos < total_len) {
            while (pos < total_len && text[pos] == ' ') pos++;
            size_t remaining = total_len - pos;
            if (remaining == 0) break;

            if (remaining <= max_chars) {
                if (n_splits >= splits_cap) {
                    splits_cap = splits_cap ? splits_cap * 2 : 16;
                    splits = realloc(splits, splits_cap * sizeof(size_t));
                }
                splits[n_splits++] = pos;
                break;
            }

            size_t window_end = pos + max_chars;
            if (window_end > total_len) window_end = total_len;

            size_t split = 0;
            int found = 0;

            // sentence
            for (size_t c = window_end; c > pos && !found; c--) {
                if (is_sentence_end(text[c - 1])) {
                    split = c;
                    found = 1;
                }
            }
            // phrase
            if (!found) {
                for (size_t c = window_end; c > pos && !found; c--) {
                    if (text[c - 1] == ',' || text[c - 1] == ';' || text[c - 1] == ':') {
                        split = c;
                        found = 1;
                    }
                }
            }
            // word
            if (!found) {
                for (size_t c = window_end; c > pos && !found; c--) {
                    if (text[c] == ' ') {
                        split = c;
                        found = 1;
                    }
                }
            }
            if (!found) split = window_end;

            if (n_splits >= splits_cap) {
                splits_cap = splits_cap ? splits_cap * 2 : 16;
                splits = realloc(splits, splits_cap * sizeof(size_t));
            }
            splits[n_splits++] = pos;
            pos = split;
        }

        // split chunks proportionally based on their length
        // and snap to whisper timestamps
        for (size_t s = 0; s < n_splits; s++) {
            size_t chunk_start = splits[s];
            size_t chunk_end;
            if (s + 1 < n_splits) {
                chunk_end = splits[s + 1];
                while (chunk_end > chunk_start && text[chunk_end - 1] == ' ')
                    chunk_end--;
            } else {
                chunk_end = total_len;
            }
            if (chunk_end <= chunk_start) continue;

            double frac_start = (double)chunk_start / total_len;
            double frac_end = (s + 1 < n_splits)
                ? (double)splits[s + 1] / total_len
                : 1.0;

            size_t bi_start = (size_t)(frac_start * (n_boundaries - 1) + 0.5);
            size_t bi_end   = (size_t)(frac_end * (n_boundaries - 1) + 0.5);
            if (bi_start >= n_boundaries) bi_start = n_boundaries - 1;
            if (bi_end >= n_boundaries) bi_end = n_boundaries - 1;
            if (bi_end <= bi_start) bi_end = bi_start + 1;
            if (bi_end >= n_boundaries) bi_end = n_boundaries - 1;

            int64_t t0 = times[bi_start];
            int64_t t1 = times[bi_end];

            // damn
            if (t1 <= t0)
                t1 = t0 + 1;

            push_subtitle(output, t0, t1, text + chunk_start,
                          chunk_end - chunk_start);
        }

        free(splits);
        free(times);
    }

    // fix timestamp conflicts
    for (size_t i = 1; i < output->count; i++) {
        if (parse_ts(output->segments[i].t0) == parse_ts(output->segments[i - 1].t0)) {
            conflict_resolve(&output->segments[i], &output->segments[i - 1]);
        }
    }
    return output;
}