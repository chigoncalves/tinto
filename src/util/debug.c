#include "conf.h" // For system checks.

#include <stdarg.h> // For `va_list` and `va_start`.
#include <stdbool.h> // For `bool`, `true` and `false`.
#include <stdio.h> // For `fprintf`, `fputc`, `stderr` and `stdio`.
#include <stdlib.h> // For `exit` and `EXIT_FAILURE`.
#include <string.h> // For `strlen` and `strncpy`.

#include <libgen.h> // For `basename` and `dirname`.

#include "debug.h"

#define BUFF_SZ 256

static const char* path_shortify (const char* path);
static void _common (const char* fmt, va_list rest);

void warn (const char* fname, int linum, const char* fmt, ...) {
  const char* path = path_shortify (fname);
  fprintf (stderr, "[WARNING @ %s:%d] - ", path, linum);

  va_list rest;
  va_start (rest, fmt);
  /* _common (fmt, rest); */
  fprintf (stderr, fmt, rest);;
  fputc ('\n', stderr);
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
  fprintf (stderr, fmt, rest);;
  fputc ('\n', stderr);
  fflush (stderr);
}

static const char* path_shortify (const char* path) {
  if (!path) return NULL;
  static char buff[BUFF_SZ];
  const size_t len = strlen (path) + 1;
  if (len > BUFF_SZ) return path;

  strncpy (buff, path, len);
  char* bname = basename (buff);
  char* dname = basename (dirname (buff));
  sprintf (buff, "%s/%s", dname, bname);

  return buff;
}
