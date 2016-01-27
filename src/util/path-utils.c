#include "conf.h"

#include <unistd.h> // For `uid_t' ant `getuid'.
#include <pwd.h> // For `getpwuid'.

#include <stddef.h> // For `size_t`.
#include <stdio.h>
#include <stdlib.h> // For `realloc'.
#include <string.h> // For `strlen', `strncat', `strncpy' and `strdup'.

#include "debug.h"
#include "path-utils.h"
#include "string-addins.h"

#define BUFFER_SZ 128U

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

char*
path_current_user_home (void) {
  struct passwd* pwd = getpwuid (getuid ());
  if (!pwd) return NULL;

  return strdup (pwd->pw_dir);
}


char* path_unexpand_tilde (const char *path) {
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
#ifdef TINTO_DEVEL_MODE
      WARN ("Bytes read mismatched bytes wrote");
#endif // TINTO_DEVEL_MODE
    }
  }

  fclose (fileDest);
  fclose (fileSrc);
}
