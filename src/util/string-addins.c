#include "conf.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "string-addins.h"

char*
strltrim (char* str) {
  if (!str) return NULL;

  char* ptr = str;
  while (*ptr) {
    if (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') break;

    ++ptr;
  }

  const size_t new_len = strlen (ptr) + 1;
  for (size_t i = 0; i < new_len; ++i) {
    str[i] = *ptr;
    ++ptr;
  }
  str[new_len] = NUL;

  return str;
}

char*
strrtrim (char* str) {
  if (!str) return NULL;
  char* ptr = str + strlen (str) - 1;

  while (*ptr == '\t' || *ptr == '\n' || *ptr == ' ') {
    --ptr;
  }
  *++ptr = NUL;

  return str;
}

char*
strtrim (char* str) {
  return strltrim (strrtrim (str));
}

bool
strendswith (const char* restrict word, const char* restrict suffix) {
  if (!word || !suffix) return NULL;

  const size_t word_len = strlen (word);
  const size_t suff_len = strlen (suffix);

  if (word_len == suff_len) return strcmp (word, suffix) == 0;
  else if (word_len > suff_len) return strcmp (word + (word_len - suff_len), suffix) == 0;
  else return strcmp (suffix + (suff_len - word_len), word) == 0;
}
