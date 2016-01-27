#ifndef TINTO_SRC_UTILS_PATH_UTILS
#define TINTO_SRC_UTILS_PATH_UTILS 1

char*
path_expand_tilde (const char* str);

char*
path_current_user_home (void);

char*
path_unexpand_tilde (const char* path);

/*!
 * \brief Copy the contents of a file.
 *
 * \param src source file.
 *
 * \param dest destination file.
 */
void
path_copy_file (const char *pathSrc, const char *pathDest);


const char*
path_shortify (const char* path);


#endif // TINTO_SRC_UTILS_PATH_UTILS
