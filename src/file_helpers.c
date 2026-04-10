//
// Created by sams on 3/4/26.
//

#include "file_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <curl/curl.h>

static int download_with_curl(const char *url, const char *dest_path)
{
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(dest_path, "wb");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "download_with_curl: %s\n", curl_easy_strerror(res));
        remove(dest_path); // cleanup partial file
        return -1;
    }

    return 0;
}

int curl_if_not_present(const char *rel_path, const char *url){
    const char *home = getenv("HOME");
    if(!home){
        fprintf(stderr, "curl_if_not_present: $HOME is not set\n");
        return -1;
    }

    char full_path[PATH_MAX];
    int written = snprintf(full_path, sizeof(full_path), "%s/%s", home, rel_path);
    if(written < 0 || (size_t)written >= sizeof(full_path)){
        fprintf(stderr, "curl_if_not_present: path too long\n");
        return -1;
    }

    if(file_exists(full_path)){
        return 0;  // already present
    }

    mkdirs_for_file(full_path);

    fprintf(stdout, "Downloading %s from:\n  %s\n", rel_path, url);
    fflush(stdout);

    if(download_with_curl(url, full_path) != 0){
        remove(full_path);
        fprintf(stderr, "curl_if_not_present: failed to download '%s'\n", rel_path);
        return -1;
    }

    fprintf(stdout, "\nSaved to: %s\n", full_path);
    return 0;
}

void mkdirs_for_file(const char *filepath){
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", filepath);
    for(char *p=tmp+1;*p;p++){
        if(*p=='/'){
            *p='\0';
            mkdir(tmp, 0755);
            *p='/';
        }
    }
}

int file_exists(const char *path){
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

int is_valid_option(const char *name, const char* option_list[]){
    if (name == NULL) return -1;
    for (int i = 0; ; i++) {
        const char *opt = option_list[i];
        if (is_end_of_array(opt)) {
            return -1;
        }
        if (opt == NULL) continue;
        if (strncmp(opt, name, 20) == 0) return i;
    }
    return -1;
}