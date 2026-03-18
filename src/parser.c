//
// Created by sams on 3/4/26.
//
#include "argp.h"
#include "parser.h"
#include "defaults.h"
#include <stdlib.h>
#include <string.h>
#include "file_helpers.h"

// argp metadata
const char *argp_program_version = "SubOptimal 1.0";
const char *argp_program_bug_address = "Suciu.Se.An@student.utcluj.ro";

static char doc[] =
    "Transcribe audio from video files using whisper.cpp\n"
    "\n"
    "Run modes:\n"
    "   --healdess  Run without GUI; read options from command line/config\n"
    "   --gui       Launch the graphical interface (default when no mode is given)\n"
    "\n"
    "In headless mode the lookup order for options is:\n"
    "   1. ~/.local/share/SubOptimal/config.yaml    (or --config path)\n"
    "   2. Command-line flags (override config values)\n";

static char args_doc[] = "";

static int is_valid_option(const char *name, const char* option_list[]){
    for (int i = 0; option_list[i]; i++)
        if (strncmp(option_list[i], name, 20) == 0) return 1;
    return 0;
}

static struct argp_option options[] = {
    /* mode */
    {"headless", 'H', NULL,      0, "Run in headless (no-GUI) mode"},
    {"gui",      'G', NULL,      0, "Launch the GUI (ignores all other options)"},
    /* headless options */
    {"config",   'c', "FILE", 0, "Path to YAML config file (headless only)"},
    {"model",    'm', "MODEL",0, "Whisper.cpp model name (e.g., ggml-base.en.bin)"},
    {"source",   's', "FILE", 0, "Path to input mp4 file to transcribe"},
    {"output",   'o', "FILE", 0, "Path to output SRT file"},
    {"translate",   't', "MODEL",  0, "Language to translate to"},
    {0}
};

// YAML STUFF
/*
 * Remove whitespace at beginning and end of line
*/
static char *trim(char *s){
    while(*s==' ' || *s=='\t'){
        s++;
    }

    char *e = s+strlen(s)-1;
    while(e>s && (*e==' ' || *e=='\t' || *e=='\r' || *e=='\n')){
        *e = '\0';
        e--;
    }

    return s;
}

/*
 * Some arguments may get stored on the heap
 * but since they will always last the lifetime of the program,
 * it makes no sense to free them. There is no drawback to leaking
 * memory here.
*/
static void load_yaml_config(const char *path, arguments* args){
    FILE *f=fopen(path,"r");
    if(!f) return;

    char line[1024];
    while(fgets(line, sizeof line, f)){
        char *p = trim(line);
        if(*p == '#' || *p == '\0') continue;

        char *colon = strchr(p, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = trim(p);
        char *val = trim(colon + 1);

        /* strip optional surrounding quotes */
        size_t vlen = strlen(val);
        if(vlen >= 2 &&
            ((val[0] == '"'  && val[vlen-1] == '"') ||
            (val[0] == '\'' && val[vlen-1] == '\''))) {
                val[vlen-1] = '\0';
                val++;
        }

        if      (strcmp(key, "model")  == 0 && !args->model)  args->model  = strdup(val);
        else if (strcmp(key, "source") == 0 && !args->source) args->source = strdup(val);
        else if (strcmp(key, "output") == 0 && !args->output) args->output = strdup(val);
    }
    fclose(f);
}

// CONFIG FILE STUFF
static void create_default_config(const char *path){
    mkdirs_for_file(path);
    FILE *f = fopen(path, "w");
    if(!f){
        fprintf(stderr, "Warning: could not create default config at %s: %s\n",
                path, strerror(errno));
        return;
    }
    fprintf(f,
        "# SubOptimal default configuration\n"
        "# Edit these values or override them on the command line.\n"
        "\n"
        "model:  %s\n"
        "source: \"\"\n"
        "output: %s\n",
        DEFAULT_MODEL, DEFAULT_OUTPUT);
    fclose(f);
}

static error_t parse_opt(int key, char *arg, struct argp_state *state){
    arguments *arguments = state->input;

    switch (key) {
        case 'H':
            arguments->mode = MODE_HEADLESS;
            break;
        case 'G':
            arguments->mode = MODE_GUI;
            break;
        case 'c':
            arguments->config = arg;
            break;
        case 'm':
            arguments->model = arg;
            break;
        case 't':
            arguments->translate = arg;
            break;
        case 's':
            arguments->source = arg;
            break;
        case 'o':
            arguments->output = arg;
            break;

        case ARGP_KEY_END:
            if(arguments->mode == MODE_UNSET){
                arguments->mode = MODE_GUI;
            }

            if(arguments->mode == MODE_GUI){
                printf("GUI not implemented yet. Run with --headless\n");
                exit(1);
            }

            char default_cfg[4096] = {0};
            if(!arguments->config) {
                const char *home = getenv("HOME");
                if(home){
                    snprintf(default_cfg,sizeof(default_cfg),
                             "%s" CONFIG_DIR "config.yaml", home);
                }

            }
            const char *cfg_path = arguments->config
                                 ? arguments->config
                                 : (default_cfg[0] ? default_cfg : NULL);

            // create config if it doesn't exist
            if(cfg_path){
                FILE *probe = fopen(cfg_path, "r");
                if(probe){
                    fclose(probe);
                } else {
                    create_default_config(cfg_path);
                }

                load_yaml_config(cfg_path, arguments);
            }

            // defaults if some error appeared
            if (!arguments->model)  arguments->model  = DEFAULT_MODEL;
            if (!arguments->output) arguments->output = DEFAULT_OUTPUT;

            if (!arguments->source || arguments->source[0] == '\0') {
                argp_error(state,
                    "Missing required option --source "
                    "(provide it on the command line or set 'source' in config)");
            }

            // validate model
            if (!is_valid_option(arguments->model,VALID_MODELS)) {
                char msg[512] = "Unknown model '";
                strncat(msg, arguments->model, sizeof(msg) - strlen(msg) - 1);
                strncat(msg, "'. Valid models are:\n", sizeof(msg) - strlen(msg) - 1);

                for (int i = 0; VALID_MODELS[i]; i++) {
                    if(i % 2 == 1){
                        strncat(msg, ", ", sizeof(msg) - strlen(msg) - 1);
                    }
                    strncat(msg, VALID_MODELS[i], sizeof(msg) - strlen(msg) - 1);
                    if (i > 0 && (i + 1) % 2 == 0) {
                        if(VALID_MODELS[i+1]){
                            strncat(msg, ",", sizeof(msg) - strlen(msg) - 1);
                        }
                        strncat(msg, "\n", sizeof(msg) - strlen(msg) - 1);
                    }
                }

                argp_error(state, "%s", msg);
            }

            // ensure translate model directory exists
            const char *home = getenv("HOME");
            if(home){
                char translate_dir[PATH_MAX];
                snprintf(translate_dir, sizeof(translate_dir), "%s%s", home, TRANSLATE_MODEL_DIR);
                mkdirs_for_file(translate_dir);
            }

            // validate translate language
            if (arguments->translate && !is_valid_option(arguments->translate, VALID_LANGUAGES)) {
                fprintf(stderr, "Unknown language '%s'. Valid languages:\n", arguments->translate);

                // print in columns: code - name
                for (int i = 0; VALID_LANGUAGES[i]; i++) {
                    fprintf(stderr, "  %s - %s\n", VALID_LANGUAGES[i], LANGUAGE_NAMES[i]);
                }

                exit(EXIT_FAILURE);
            }
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {
    .options  = options,
    .parser   = parse_opt,
    .args_doc = args_doc,
    .doc      = doc,
};

arguments* parse_args(const int argc, char *argv[]) {
    arguments* arguments = malloc(sizeof(*arguments));
    if (!arguments) {
        perror("parse_args: malloc");
        exit(EXIT_FAILURE);
    }

    // initialize all fields to NULL
    arguments->model = NULL;
    arguments->source = NULL;
    arguments->output = NULL;
    arguments->config = NULL;
    arguments->translate = NULL;
    arguments->mode = MODE_UNSET;

    if (argp_parse(&argp, argc, argv, 0, NULL, arguments) != 0)
        exit(EXIT_FAILURE);

    // discard this stuff in gui mode
    if (arguments->mode == MODE_GUI) {
        arguments->model  = NULL;
        arguments->source = NULL;
        arguments->output = NULL;
        arguments->config = NULL;
    }

    return arguments;
}