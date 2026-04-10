//
// Created by sams on 3/18/26.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H
#include "subtitles.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Initialize translator.
     * model_path - path to model directory
     * spm_path   - path to sentencepiece model (.spm file)
     */
    void translator_init(const char *model_path, const char *spm_path);

    /*
     * Translate text from source to target language.
     * Returns allocated string (caller must free).
     */
    char *translator_translate(const char *text, const char *source, const char *target);

    /*
     * Translate a subtitle list from source to target language.
     * Performs in-place translation
     */
    void translate_subtitles(subtitle_list* subtitles, const char *source, const char *target);

    void translator_free(void);

#ifdef __cplusplus
}
#endif

#endif //TRANSLATE_H
