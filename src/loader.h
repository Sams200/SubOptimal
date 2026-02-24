//
// Created by sams on 2/24/26.
//

#ifndef LOADER_H
#define LOADER_H
#include <stddef.h>

unsigned char* load_audio_via_ffmpeg(const char* video_path, size_t* size_out);

#endif //LOADER_H
