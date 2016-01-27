#include "conf.h"

#include <unistd.h> // For `uid_t' ant `getuid'.
#include <pwd.h> // For `getpwuid'.

#include <stddef.h> // For `size_t'.
#include <stdlib.h> // For `realloc'.
#include <string.h> // For `strlen', `strncat', `strncpy' and `strdup'.

#include "path-utils.h"
#include "string-addins.h"

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
