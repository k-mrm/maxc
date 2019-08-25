#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "literalpool.h"
#include "maxc.h"
#include "mem.h"
#include "object.h"
#include "frame.h"

extern MxcObject **stackptr;
extern Vector *ltable;

typedef struct _VM {
    Frame *frame;
    Vector *gvmap;
    Vector *framestack;
} VM;

VM *New_VM(Bytecode *, int ngvar);

int VM_run(VM *);

// stack
#define Push(ob) (*stackptr++ = ((MxcObject *)(ob)))
#define Pop() (*--stackptr)
#define Top() (stackptr[-1])
#define SetTop(ob) (stackptr[-1] = ob)

#endif
