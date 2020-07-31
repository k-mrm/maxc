#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "frame.h"
#include "util.h"

extern Vector *ltable;
extern int error_flag;

typedef struct VM VM;
struct VM {
  MContext *ctx;
  MxcValue *gvars;
  size_t ngvars;
  MxcValue *stackptr;
  MxcValue *stackbase;
};

VM *new_vm(Bytecode *, int);
int vm_run(VM *);
int vm_exec(VM *);
void stack_dump(void);

#define Push(ob) (*frame->stackptr++ = (ob))
#define Pop() (*--frame->stackptr)
#define Top() (frame->stackptr[-1])
#define SetTop(ob) (frame->stackptr[-1] = (ob))

#endif
