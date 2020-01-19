#ifndef MAXC_H
#define MAXC_H

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

int mxc_main(const char *, const char *);
int mxc_main_repl(void);

typedef struct MxcArg {
    int argc;
    char **argv;
} MxcArg;

#endif
