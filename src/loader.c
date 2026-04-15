//
// Created by sams on 2/24/26.
//

#include "loader.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DR_WAV_IMPLEMENTATION
#include "../drwav/dr_wav.h"

static unsigned char* load_audio_via_ffmpeg(const char* video_path, size_t* size_out){
    int pipefd[2]; // 0 - read; 1 - write
    pid_t pid;

    if(pipe(pipefd) < 0){
        perror("LOADER: Could not open pipe");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(1);
    }

    pid = fork();
    if(pid < 0){
        perror("LOADER: Could not fork process");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(1);
    }

    if(pid == 0){
        // CHILD
        // call ffmpeg to extract sound
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // ac means 1 channel (mono) since whisper wants that
        // also include a filter to remove background music
        execlp("ffmpeg", "ffmpeg",
            "-loglevel", "warning",
            "-i", video_path,
            // "-af", "highpass=f=150,lowpass=f=4500",
            "-f", "wav",
            "-ar", "16000",
            "-ac", "1",
            "-", NULL);

        // should not reach here
        perror("LOADER: Could not execute ffmpeg");
        exit(1);
    }

    // PARENT
    close(pipefd[1]);

    // expanding buffer
    // may change later to do segment by segment
    size_t capacity = 1024*1024;
    size_t size = 0;

    unsigned char* buffer = malloc(capacity);
    if(buffer == NULL){
        perror("malloc");
        return NULL;
    }

    size_t bytes_read;
    while((bytes_read = read(pipefd[0], buffer+size, capacity-size)) > 0){
        size += bytes_read;

        if(size>=capacity){
            capacity *= 2;
            unsigned char* temp = realloc(buffer, capacity);
            if(temp == NULL){
                perror("realloc");
                return NULL;
            }
            buffer = temp;
        }
    }
    close(pipefd[0]);

    waitpid(pid, NULL, 0);
    *size_out = size;
    return buffer;
}

float* load_audio(const char* source, size_t* frame_count_out){
    printf("Loading audio from: %s\n", source);
    fflush(stdout);

    size_t wav_size;
    unsigned char* wav_data = load_audio_via_ffmpeg(source, &wav_size);

    if(!wav_data){
        fprintf(stderr, "load_audio: Failed to load audio via ffmpeg\n");
        return NULL;
    }

    drwav wav;
    if(!drwav_init_memory(&wav, wav_data, wav_size, NULL)){
        fprintf(stderr, "load_audio: Failed to initialize drwav\n");
        free(wav_data);
        return NULL;
    }

    if(wav.channels != 1){
        fprintf(stderr, "load_audio: WAV channels must be 1\n");
        drwav_uninit(&wav);
        free(wav_data);
        return NULL;
    }

    float* pcm_frames = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(float));
    if(!pcm_frames){
        fprintf(stderr, "load_audio: Failed to allocate PCM buffer\n");
        drwav_uninit(&wav);
        free(wav_data);
        return NULL;
    }

    drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pcm_frames);
    *frame_count_out = wav.totalPCMFrameCount;

    drwav_uninit(&wav);
    free(wav_data);

    printf("Successfully loaded audio\n");
    fflush(stdout);
    return pcm_frames;
}
