// No header guards since it is legitimately possible to include this file more
// than once with and without NDEBUG.

#undef assert
#undef _assert

#ifdef NDEBUG

#define assert(e) ((void)0)
#define _assert(e) ((void)0)

#else // NDEBUG

#ifdef __cplusplus
extern "C" {
#endif
void hermes_assert_fail(const char *file, int line, const char *expr);
#ifdef __cplusplus
}
#endif

#define assert(e) ((e) ? (void)0 : hermes_assert_fail(__FILE__, __LINE__, #e))
#define _assert(e) assert(e)

#endif // NDEBUG
