#include "conf.h" // For system checks.

#include <stdarg.h> // For `va_list` and `va_start`.
#include <stdio.h> // For `fprintf`, `fputc`, `stderr`, `vfprintf' and `stdout`,.
#include <stdlib.h> // For `exit` and `EXIT_FAILURE`.

#include "debug.h"
#include "path-utils.h"

static void _common (const char* fmt, va_list rest);

void warn (const char* fname, int linum, const char* fmt, ...) {
  const char* path = path_shortify (fname);
  fprintf (stderr, "[WARNING @ %s:%d] - ", path, linum);

  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
}

void die (const char* fname, int linum, const char* fmt, ...)  {
  const char* path = path_shortify (fname);
  fprintf (stderr, "[DIE @ %s:%d] - ", path, linum);

  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
  exit (EXIT_FAILURE);
}

void msg (const char* fmt, ...) {
  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
}

static void _common (const char* fmt, va_list rest)  {
  vfprintf (stderr, fmt, rest);;
  fputc ('\n', stderr);
  fflush (stderr);
}
