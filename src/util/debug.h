#ifndef SRC_UTIL_DEBUG
#define SRC_UTIL_DEBUG

#ifdef NDEBUG
#define DEBUG(...)
#define DIE(...)
#else
#define DEBUG(...) debug (__FILE__, __LINE__, __VA_ARGS__)
#define DIE(...) die (__FILE__, __LINE__, __VA_ARGS__)
#endif // NDEBUG

#define MESSAGE(...) msg (__VA_ARGS__)

void debug (const char* fname, int linum, const char* fmt, ...);
void die (const char* fname, int linum, const char* fmt, ...);
void msg (const char* fmt, ...);

#endif // SRC_UTIL_DEBUG
