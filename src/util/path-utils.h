/*! \file path-utils.h
 * Path handling utility functions.
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
