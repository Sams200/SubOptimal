#ifndef CANCEL_H
#define CANCEL_H

#ifdef __cplusplus
extern "C" {
#endif

void cancel_reset(void);
void cancel_signal(void);
int is_cancelled(void);

#ifdef __cplusplus
}
#endif

#endif // CANCEL_H
