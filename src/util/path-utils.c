/*! \file path-utils.c
 * Path handling utility functions.
 */
#include "conf.h"

#include <libgen.h>    // For `basename` and `dirname`.
#include <unistd.h>    // For `uid_t' ant `getuid'.
#include <pwd.h>       // For `getpwuid'.

#include <stddef.h>    // For `size_t`.
#include <stdio.h>     // For `fread', `feof', `fwrite', `sprintf', `fopen' and `fclose'.
#include <stdlib.h>    // For `realloc'.
#include <string.h>    // For `strlen', `strncat', `strncpy' and `strdup'.

#include "debug.h"
#include "path-utils.h"
#include "string-addins.h"

#define BUFFER_SZ 256U

/*!
 * \brief Expand tilde (~) in a string to the user home directory.
 *
 * \param str a string to be expanded.
 *
 * \return a string with tilde expanded, the string should be later on
 * freed by free ().
 */
char*
path_expand_tilde (const char* str) {

  if (strstartswith (str, "~") || strstartswith (str, "~/")) {
    char* home_dir = path_current_user_home ();
    if (!home_dir) return NULL;

    const size_t len_str = strlen (str);
    char* aux = realloc (home_dir, strlen (home_dir) + len_str);
    if (!aux) {
      free (home_dir);
      return NULL;
    }

    home_dir = aux;
    aux = NULL;

    strncat (home_dir, str + 1, len_str);

    return home_dir;
  }

  return strdup (str);
}

/*!
 * \brief Get the current user home directory.
 *
 * \par
 * The string returned should be friend later on.
 */
char*
path_current_user_home (void) {
  struct passwd* pwd = getpwuid (getuid ());
  if (!pwd) return NULL;

  return strdup (pwd->pw_dir);
}

/*!
 * \brief Unexpand tilde (~) in a string to the user home directory.
 *
 * \param path a file path.
 *
 */
char*
path_unexpand_tilde (const char *path) {
  char *home = path_current_user_home ();
  if (!home) return NULL;
  else if (!strstartswith (path, home)) return strdup (path);

  const size_t home_len = strlen (home);
  const size_t new_len = strlen (path + home_len) + 1; // For 1+ "~".

  if (home_len != new_len) {
    char* aux = realloc (home, new_len);
    if (!aux) {
      free (home);
      return NULL;
    }

    home = aux;
    aux = NULL;
  }

  strncpy (home, "~", 2);
  strncat (home, path + home_len, new_len - 1);

  return home;
}


/*!
 * \brief Copy the contents of a file.
 *
 * \param pathSrc source file.
 *
 * \param pathDest destination file.
 */
void
path_copy_file (const char *pathSrc, const char *pathDest) {
  FILE* fileSrc = fopen(pathSrc, "rb");
  if (!fileSrc) return;

  FILE* fileDest = fopen(pathDest, "wb");
  if (!fileDest) {
    fclose (fileSrc);
    return;
  }

  char buffer[BUFFER_SZ];
  size_t bytes = 0;

  while (feof (fileSrc) == 0) {
    bytes = fread (buffer, 1U, BUFFER_SZ, fileSrc);

    if (bytes != fwrite (buffer, 1, bytes, fileDest)) {
      WARN ("Bytes read mismatched bytes wrote");
    }
  }

  fclose (fileDest);
  fclose (fileSrc);
}

/*!
 * \brief Shortify ta a path name.
 *
 * \param path a path name.
 *
 * \return a short path name.
 *
 * \note This function is note thread safe.
 */
const char*
path_shortify (const char* path) {
  if (!path) return NULL;
  static char buff[BUFFER_SZ];
  const size_t len = strlen (path) + 1;
  if (len > BUFFER_SZ) return path;

  strncpy (buff, path, len);
  char* bname = basename (buff);
  char* dname = basename (dirname (buff));
  sprintf (buff, "%s/%s", dname, bname);

  return buff;
}
