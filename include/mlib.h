#ifndef MXC_MLIB_H
#define MXC_MLIB_H

#include "maxc.h"
#include "mlibapi.h"

void std_init(MInterp *);
void flib_init(MInterp *);
void strlib_init(MInterp *);
void listlib_init(MInterp *);
void load_default_module(MInterp *);
void register_module(MInterp *, MxcModule *);

#define FTYPE(...) __VA_ARGS__, NULL

#endif
