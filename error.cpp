#include "maxc.h"

extern char *filename;
extern bool iserror;

void error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\e[31;1m[error] \e[0m");
    if(filename)
        fprintf(stderr, "\e[1m%s:\e[0m\n", filename);
    vfprintf(stderr, msg, args); puts("");
    va_end(args);

    iserror = true;
}
