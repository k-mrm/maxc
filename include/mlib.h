#ifndef MXC_MLIB_H
#define MXC_MLIB_H

#include "maxc.h"
#include "mlibapi.h"

MxcModule *std_module(void);
MxcModule *flib_module(void);
MxcModule *strlib_module(void);
MxcModule *listlib_module(void);
void load_default_module();

void setup_argv(int argc, char **argv);

extern Vector *gmodule;

#define FTYPE(...) __VA_ARGS__, NULL

#endif
