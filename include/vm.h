#ifndef MAXC_VM_H
#define MAXC_VM_H

#include <setjmp.h>
#include "maxc.h"
#include "context.h"
#include "util.h"
#include "literalpool.h"

#define DIRECT_THREADED

extern int error_flag;

typedef struct VM VM;
struct VM {
  MContext *ctx;
  MxcValue *gvars;
  size_t ngvars;
  MxcValue *stackptr;
  MxcValue *stackbase;
  Vector *ltable;

  jmp_buf vm_end_jb;
};

extern VM gvm;  /* global VM */

void vm_open(mptr_t *, MxcValue *, int, Vector *, DebugInfo *);
void vm_force_exit(int);
int vm_run(void);
void *vm_exec(VM *);
void stack_dump(char *);

static inline VM *curvm() {
  return &gvm;
}

#define PUSH(ob) (*vm->stackptr++ = (ob))
#define POP() (*--vm->stackptr)
#define TOP() (vm->stackptr[-1])
#define SETTOP(ob) (vm->stackptr[-1] = (ob))

#endif
