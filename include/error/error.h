#include "maxc.h"
#include "token.h"

#ifdef MXC_DEBUG
#define mxc_assert(expr, msg) mxc_assert_core(expr, msg, __FILE__, __LINE__)
#else
#define mxc_assert(expr, msg)
#endif

void error(const char *, ...);
void errline(int, char *, ...);
void warn(const char *, ...);
void error_at(const SrcPos, const SrcPos, const char *, ...);
void mxc_unimplemented(const char *, ...);
void expect_token(const SrcPos, const SrcPos, const char *);
void unexpected_token(Token *tk, ...);
void warning(const SrcPos, const SrcPos, const char *, ...);
void debug(const char *, ...);
void showline(int, int);

void mxc_assert_core(int, char *, char *, int);
