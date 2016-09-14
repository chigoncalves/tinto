/*!
 * \file string-addins.h
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


#ifndef TINTO_SRC_UTIL_STRING_ADDINS
#define TINTO_SRC_UTIL_STRING_ADDINS 1

#include <stdbool.h>

/*! Trim a string on the left. */
char*
strltrim (char* str);

/*! Trim the string on the right side. */
char*
strrtrim (char* str);

/*! Trim a string on the both sides. */
char*
strtrim (char* str);

/*! Check if a string starts with a suffix. */
bool
strendswith (const char* word, const char* suffix);

/*! Check if a string ends with a suffix. */
bool
strstartswith (const char* restrict word, const char* restrict prefix);

#endif // TINTO_SRC_UTIL_STRING_ADDINS
