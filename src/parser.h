//
// Created by sams on 3/4/26.
//

#ifndef PARSER_H
#define PARSER_H

#define DEFAULT_MODEL  "ggml-base.en.bin"
#define DEFAULT_OUTPUT "./output.srt"

// Run mode: headless (CLI/config) or GUI
typedef enum {
    MODE_UNSET,
    MODE_HEADLESS,
    MODE_GUI
} run_mode_t;

struct arguments {
    run_mode_t  mode;
    const char *model;
    const char *source;
    const char *output;
    const char *config;   // explicit --config path (headless only)
};

extern struct arguments arguments;

/*
 * Parse arguments and place them in the arguments global variable.
 * Exits the process on error.
 */
void parse_args(int argc, char *argv[]);

#endif //PARSER_H
