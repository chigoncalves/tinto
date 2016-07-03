#ifndef TINTO_SRC_TINTO_H
#define TINTO_SRC_TINTO_H 1

#include <stdnoreturn.h>

noreturn void
tinto_usage (void);

void
tinto_signal_handler(int sig);

void
tinto_init (int argc, char *argv[]);

void
tinto_deinit (void);

void
tinto_take_snapshot(const char *path);

#endif // TINTO_SRC_TINTO_H
