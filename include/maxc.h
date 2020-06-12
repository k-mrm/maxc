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

MInterp _mxc_global_interp;

#define our_interp() (&_mxc_global_interp)

int mxc_main_file(const char *);
int mxc_main_repl(void);

void mxc_interp_open(int, char **);

#endif
