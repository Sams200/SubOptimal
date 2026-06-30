//
// Created by sams on 2/24/26.
//

#ifndef LOADER_H
#define LOADER_H
#include <stddef.h>

/*
 * Load audio from file and decode to mono float PCM frames.
 * Uses ffmpeg to extract audio and dr_wav to decode to float PCM.
 *
 * source   - path to input audio/video file
 * size_out - output parameter for number of PCM frames
 *
 * Returns allocated float array of PCM frames (caller must free), or NULL on error.
 */
float* load_audio(const char* source, size_t* size_out);

#endif //LOADER_H
