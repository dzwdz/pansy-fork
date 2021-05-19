// this header file can be included multiple times
#undef assert

#ifdef NDEBUG
#   define assert(x) ((void)0)
#else
#   define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__), 0)))
#endif

void __assert_fail(const char *expr, const char *file, int line, const char *func);
