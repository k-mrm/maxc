#include "maxc.h"

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error] \e[0m");
    vfprintf(stderr, msg, args); puts("");
    va_end(args);
}
