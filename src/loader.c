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

unsigned char* load_audio_via_ffmpeg(const char* video_path, size_t* size_out){
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
        execlp("ffmpeg", "ffmpeg", "-loglevel", "warning", "-i", video_path, "-f", "wav", "-ar", "16000", "-ac", "1", "-", NULL);

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
