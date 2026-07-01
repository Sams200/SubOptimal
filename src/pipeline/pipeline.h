//
// Created by sams on 7/1/26.
//

#ifndef PIPELINE_H
#define PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subtitles/subtitles.h"

    subtitle_list* perform_transcribe(const char *model, const char *source,
                                      const char *language, int *error);
    void perform_translate(subtitle_list *list, const char *target_nllb, int *error);
    int write_subtitles_to_file(const char *output_path, const subtitle_list *list);

#ifdef __cplusplus
}
#endif

#endif // PIPELINE_H
