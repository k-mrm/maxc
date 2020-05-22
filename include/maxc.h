#ifndef MAXC_H
#define MAXC_H

#include "internal.h"
#include "util.h"

int mxc_main(const char *, const char *);
int mxc_main_repl(void);

typedef struct MxcArg {
  int argc;
  char **argv;
} MxcArg;

typedef struct Interp Interp;
struct Interp {
};

#endif
