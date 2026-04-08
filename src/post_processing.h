//
// Created by sams on 4/8/26.
//

#ifndef POST_PROCESSING_H
#define POST_PROCESSING_H

#include <stddef.h>
#include "subtitles.h"

// Timestamps with unfinished sentences together
merged_list* merge_sentences(subtitle_list *input);

// Split timestamps according to sentence > phrase > word
subtitle_list* split_for_display(merged_list *merged, size_t max_chars);

#endif //POST_PROCESSING_H
