#ifndef MAXC_H
#define MAXC_H

#include "internal.h"

int mxc_main(const char *, const char *);
int mxc_main_repl(void);

typedef struct MxcArg {
    int argc;
    char **argv;
} MxcArg;

#endif
