/*!
 * \file debug.h
 * Debug utilities functions.
 */


/*!
 *
 * Copyright © 2016, Edelcides Gonçalves <eatg75 |0x40| gmail>
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 *
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
