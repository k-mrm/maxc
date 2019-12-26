#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "frame.h"
#include "literalpool.h"
#include "maxc.h"
#include "mem.h"
#include "object/object.h"

extern Vector *ltable;
extern int error_flag;

int VM_run(Frame *);
MxcObject **VM_run_repl(Frame *);

#endif
