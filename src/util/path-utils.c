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
