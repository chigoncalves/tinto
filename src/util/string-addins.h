/*!
 * \file string-addins.h
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
