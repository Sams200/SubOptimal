//
// Created by sams on 3/4/26.
//

#include "file_helpers.h"

#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>

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