#include "maxc.h"
#include "token.h"

#ifdef MXC_DEBUG
#define mxc_assert(expr, msg) mxc_assert_core(expr, msg, __FILE__, __LINE__)
#else
#define mxc_assert(expr, msg) (0)
#endif

void error(const char *, ...);
void warn(const char *, ...);
void error_at(const Location, const Location, const char *, ...);
void mxc_unimplemented(const char *, ...);
void expect_token(const Location, const Location, const char *);
void warning(const Location, const Location, const char *, ...);
void runtime_err(const char *, ...);
void debug(const char *, ...);
void showline(int, int);

void mxc_assert_core(int, char *, char *, int);
