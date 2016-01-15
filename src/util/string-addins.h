#ifndef SRC_UTIL_STRING_ADDINS
#define SRC_UTIL_STRING_ADDINS

#include <stdbool.h>

char*
strltrim (char* str);

char*
strrtrim (char* str);

char*
strtrim (char* str);

bool
strendswith (const char* word, const char* suffix);

#endif // SRC_UTIL_STRING_ADDINS
