#include "maxc.h"

#include "token.h"

enum class ErrorKind {};

void error(const char *, ...);
void error(const location_t &, const location_t &, const char *, ...);
void warning(const location_t &, const location_t &, const char *, ...);
void runtime_err(const char *, ...);
void debug(const char *, ...);
void showline(int, int);
