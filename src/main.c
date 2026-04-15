#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "cli.h"

int main(int argc, char *argv[]) {
    arguments *args = parse_args(argc, argv);

    if (args->mode == MODE_HEADLESS) {
        int ret = cli_run(args);
        free(args);
        return ret;
    }

    // MODE_GUI or MODE_UNSET
    printf("GUI not implemented yet. Run with --headless\n");
    free(args);
    return 1;
}