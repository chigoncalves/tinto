/*!
 * \file debug.h
 * Debug utilities functions.
 */

#ifndef TINTO_PANEL_SRC_UTIL_DEBUG
#define TINTO_PANEL_SRC_UTIL_DEBUG 1

#ifdef NDEBUG
#define WARN(...)
#define DIE(...)
#else
/*! Print a warning message to console. Use this macro instead of warn ()
 *  function directly. */
#define WARN(...) warn (__FILE__, __LINE__, __VA_ARGS__)

/*! Print a error message to console and terminates the application. Use this
 * macro instead of die () function directly. */
#define DIE(...) die (__FILE__, __LINE__, __VA_ARGS__)
#endif // NDEBUG

/*! Print a message to console. Use this macro instead of msg () function
 * directly.
 */
#define MSG(...) msg (__VA_ARGS__)

/*! Print a message to console. */
void
msg (const char* fmt, ...);

/*! Print a warning message to console.*/
void
warn (const char* fname, int linum, const char* fmt, ...);

/*! Print a error message to console and terminates the application. */
void
die (const char* fname, int linum, const char* fmt, ...);

#endif // SRC_UTIL_DEBUG
