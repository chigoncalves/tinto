#ifndef TINTO_SRC_UTILS_PATH_UTILS
#define TINTO_SRC_UTILS_PATH_UTILS 1


char*
path_expand_tilde (const char* str);

char*
path_current_user_home (void);

char*
path_unexpand_tilde (const char* path);


#endif // TINTO_SRC_UTILS_PATH_UTILS
