#ifndef MAXC_H
#define MAXC_H

#include <stdbool.h>
#include "internal.h"
#include "util.h"
#include "module.h"
#include "object/object.h"
#include "frame.h"

#define MXC_MOD_INITFUNC 

typedef struct MInterp MInterp;
struct MInterp {
  int argc;
  char **argv;
  Vector *module;
  MxcValue *global;
  Frame *cur_frame;
  bool is_vm_running;
  int errcnt;
};

int mxc_main_file(MInterp *, const char *);
int mxc_main_repl(MInterp *);

MInterp *mxc_open(int, char **);
void mxc_close(MInterp *);

#endif
