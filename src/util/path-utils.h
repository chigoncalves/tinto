/*! \file path-utils.h
 * Path handling utility functions.
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


#ifndef TINTO_SRC_UTILS_PATH_UTILS
#define TINTO_SRC_UTILS_PATH_UTILS 1

/*! Expand tilde (~) in a string to the user home directory. */
char*
path_expand_tilde (const char* str);

/*! Get the current user home directory. */
char*
path_current_user_home (void);

/*! Unexpand tilde (~) in a string to the user home directory. */
char*
path_unexpand_tilde (const char* path);

/*! Copy the contents of a file. */
void
path_copy_file (const char *pathSrc, const char *pathDest);

/*! Shortify ta a path name. */
const char*
path_shortify (const char* path);

#endif // TINTO_SRC_UTILS_PATH_UTILS
