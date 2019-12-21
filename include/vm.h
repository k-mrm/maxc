#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "frame.h"
#include "literalpool.h"
#include "maxc.h"
#include "mem.h"
#include "object/object.h"

extern MxcObject **stackptr;
extern Vector *ltable;
extern int error_flag;

typedef struct VM {
    MxcObject **stack;
    Frame *vm_frame;
    MxcObject **global_vars;
} VM;

VM *New_VM(Bytecode *, int);

int VM_run(VM *);

#endif
