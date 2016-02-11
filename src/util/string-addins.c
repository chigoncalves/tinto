/*!
 * \file string-addins.c
 */

#include "conf.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "string-addins.h"

/*!
 * \brief Trim a string on the left.
 *
 * \param str a string.
 *
 * \return a the trimmed string.
 */
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

/*!
 * \brief Trim the string on the right side.
 *
 * \param str a string.
 *
 * \return the trimmed string.
 */
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

/*!
 * \brief Trim a string on the both sides.
 *
 * \param str a string.
 *
 * \return a trimmed string.
 */
inline char*
strtrim (char* str) {
  return strltrim (strrtrim (str));
}

/*!
 * \brief Check if a string starts with a suffix.
 *
 * \param word target string.
 *
 * \param suffix the preffix string.
 *
 * \return true if \word starts with \preffix, false otherwise.
 */
bool
strendswith (const char* restrict word, const char* restrict preffix) {
  if (!word || !preffix) return NULL;

  const size_t word_len = strlen (word);
  const size_t suff_len = strlen (preffix);

  if (word_len == suff_len) return strcmp (word, preffix) == 0;
  else if (word_len > suff_len) return strcmp (word + (word_len - suff_len), preffix) == 0;
  else return strcmp (preffix + (suff_len - word_len), word) == 0;
}

/*!
 * \brief Check if a string ends with a suffix.
 *
 * \param word target string.
 *
 * \param suffix the suffix string.
 *
 * \return true if \word ends with \suffix, false otherwise.
 */
bool
strstartswith (const char* restrict word, const char* restrict prefix){
  if (!word || !prefix) return NULL;

  const size_t word_len = strlen (word);
  const size_t prefix_len = strlen (prefix);


  if (word_len == prefix_len) return strcmp (word, prefix) == 0;
  else if (word_len > prefix_len) return strncmp (prefix, word, prefix_len) == 0;
  else return false;
}
