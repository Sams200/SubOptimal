#include "cancel.h"
#include <stdatomic.h>

static atomic_int cancel_flag = 0;

void cancel_reset(void) {
    atomic_store(&cancel_flag, 0);
}

void cancel_signal(void) {
    atomic_store(&cancel_flag, 1);
}

int is_cancelled(void) {
    return atomic_load(&cancel_flag);
}
