//
// Created by sams on 3/4/26.
//

#include "transcribe.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "defaults.h"
#include "file_helpers.h"

#define HF_BASE_URL "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/"

static int download_with_curl(const char *url, const char *dest_path){
    pid_t pid = fork();

    if(pid < 0){
        perror("download_with_curl: fork");
        return -1;
    }

    if(pid == 0){
        // curl here
        execlp("curl", "curl",
               "-L",           // follow redirects
               "-f",           // fail on HTTP error status
               "-#",           // progress bar
               "-o", dest_path,
               url,
               (char *)NULL);

        // should not reach here
        perror("download_with_curl: execlp curl");
        _exit(127);
    }

    // parent
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("download_with_curl: waitpid");
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "curl exited with status %d\n", WEXITSTATUS(status));
        return -1;
    }

    return 0;
}

int ensure_model(const char *model_name, char *model_path, size_t path_size){
    // $HOME/.local/share/SubOptimal/<model_name>
    const char *home = getenv("HOME");
    if(!home){
        fprintf(stderr, "ensure_model: $HOME is not set\n");
        return -1;
    }

    int written = snprintf(model_path, path_size,
                           "%s" CONFIG_DIR "%s", home, model_name);
    if(written < 0 || (size_t)written>=path_size){
        fprintf(stderr, "ensure_model: model path too long\n");
        return -1;
    }

    if(file_exists(model_path)){
        return 0;
    }

    mkdirs_for_file(model_path);

    char url[512];
    written = snprintf(url, sizeof(url), "%s%s", HF_BASE_URL, model_name);
    if(written<0 || (size_t)written>=sizeof(url)){
        fprintf(stderr, "ensure_model: URL too long\n");
        return -1;
    }

    fprintf(stdout, "Model '%s' not found. Downloading from:\n  %s\n",
            model_name, url);
    fflush(stdout);

    if(download_with_curl(url, model_path) != 0){
        // Remove any partial file curl may have left behind
        remove(model_path);
        fprintf(stderr, "ensure_model: failed to download '%s'\n", model_name);
        return -1;
    }

    fprintf(stdout, "\nModel saved to: %s\n", model_path);
    return 0;
}