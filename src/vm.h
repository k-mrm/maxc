#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "frame.h"
#include "literalpool.h"
#include "maxc.h"
#include "mem.h"
#include "object.h"

extern MxcObject **stackptr;
extern Vector *ltable;

int VM_run(Bytecode *, int ngvar);

// stack
#define Push(ob) (*stackptr++ = ((MxcObject *)(ob)))
#define Pop() (*--stackptr)
#define Top() (stackptr[-1])
#define SetTop(ob) (stackptr[-1] = ob)

#endif
