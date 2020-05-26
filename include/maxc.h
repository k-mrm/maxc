#ifndef MAXC_H
#define MAXC_H

#include "internal.h"
#include "util.h"
#include "module.h"
#include "object/object.h"

#define MXC_MOD_INITFUNC 

typedef struct MxcArg {
  int argc;
  char **argv;
} MxcArg;

typedef struct Interp Interp;
struct Interp {
  int argc;
  char **argv;
  Vector *module;
  MxcValue *global;
};

Interp interp;

#define get_current_interp() (&interp)

int mxc_main(const char *, const char *);
int mxc_main_repl(void);

void mxc_interp_open(Interp *, int, char **);

#endif
