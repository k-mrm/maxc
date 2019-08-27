#include "maxc.h"
#include "token.h"

void error(const char *, ...);
void error_at(const Location, const Location, const char *, ...);
void mxc_unimplemented(const char *, ...);
void expect_token(const Location, const Location, const char *);
void warning(const Location, const Location, const char *, ...);
void runtime_err(const char *, ...);
void debug(const char *, ...);
void showline(int, int);

void mxc_assert();
