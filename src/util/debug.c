/*!
 * \file debug.c
 */

#include "conf.h"     // For system checks.

#include <stdarg.h>   // For `va_list', `va_end' and `va_start'.
#include <stdio.h>    // For `fprintf', `fputc', `stderr', `fflush' and `vfprintf'.
#include <stdlib.h>   // For `exit' and `EXIT_FAILURE'.

#if defined (__STD_C__) && __STDC_VERSION__ == 201112L
#include <stdnoreturn.h>  // For `noreturn'.
#endif //  defined (__STD_C__) && __STDC_VERSION__ == 201112L

#include "debug.h"
#include "path-utils.h"


/*! Shared code used by warn () and die () */
static void _common (const char* fmt, va_list rest);

/*!
 * \brief Prints a warning message to console.
 *
 * \param fname the current filename.
 *
 * \param linum the current file line number.
 *
 * \param fmt the printf () style format string.
 *
 * \note This function is not thread safe, because it uses path_shortify ()
 * which in turn is not thread safe.
 */
void warn (const char* fname, int linum, const char* fmt, ...) {
  const char* path = path_shortify (fname);
  fprintf (stderr, "[WARNING @ %s:%d] - ", path, linum);

  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
}

/*!
 * \brief Prints a warning message to console and then exiting the application
 * EXIT_FAILURE status code.
 *
 * \param fname the current filename.
 *
 * \param linum the current file line number.
 *
 * \param fmt the printf () style format string.
 *
 * \note This function is not thread safe, because it uses path_shortify ()
 * which in turn is not thread safe.
 */
#if defined (__STD_C__) && __STDC_VERSION__ == 201112L
noreturn void die (const char* fname, int linum, const char* fmt, ...)  {
#else
void die (const char* fname, int linum, const char* fmt, ...)  {
#endif //  defined (__STD_C__) && __STDC_VERSION__ == 201112L
  const char* path = path_shortify (fname);
  fprintf (stderr, "[DIE @ %s:%d] - ", path, linum);

  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
  exit (EXIT_FAILURE);
}

/*!
 * \brief Prints a message to console.
 *
 * \param fmt the printf () style format string.
 *
 */
void msg (const char* fmt, ...) {
  va_list rest;
  va_start (rest, fmt);
  _common (fmt, rest);
  va_end (rest);
}

/*! Shared code used by warn () and die () */
static void _common (const char* fmt, va_list rest)  {
  vfprintf (stderr, fmt, rest);;
  fputc ('\n', stderr);
  fflush (stderr);
}
