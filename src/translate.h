//
// Created by sams on 3/18/26.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H

#ifdef __cplusplus
extern "C" {
#endif

    void translator_init(const char *model_path);
    char *translator_translate(const char *text, const char *source, const char *target);
    void translator_free(void);
    int translator_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif //TRANSLATE_H
