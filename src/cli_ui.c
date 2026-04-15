//
// Created by sams on 4/11/26.
//

#include "cli_ui.h"
#include <stdio.h>

void print_progress(int percent) {
    int bar_width = 50;
    int filled = bar_width * percent / 100;

    printf("\r[");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) printf("#");
        else printf(" ");
    }
    printf("] %d%%", percent);
    fflush(stdout);
}