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

#ifdef NDEBUG
#define MXC_DEBUG
#endif

#define STR_DEFAULT "\e[0m"
#define BOLD(s) "\e[1m" s "\e[0m"
#define MUTED(s) "\e[2m" s "\e[0m"

#define MXC_VERSION "0.0.1"

int mxc_main(const char *src);
int mxc_main_repl(void);

typedef struct MxcArg {
    int argc;
    char **argv;
} MxcArg;

#endif
