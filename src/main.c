#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "cli.h"

#ifdef WITH_GUI
#include "gui.h"
#endif

int main(int argc, char *argv[]) {
    arguments *args = parse_args(argc, argv);

    if (args->mode == MODE_HEADLESS) {
        int ret = cli_run(args);
        free(args);
        return ret;
    }

    // MODE_GUI or MODE_UNSET - discard other args, launch GUI
    free(args);
#ifdef WITH_GUI
    return run_gui(argc, argv);
#else
    fprintf(stderr, "GUI mode not available (Qt6 not found). Use --headless.\n");
    return 1;
#endif
}