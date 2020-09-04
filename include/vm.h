#ifndef MAXC_VM_H
#define MAXC_VM_H

#include <setjmp.h>
#include "context.h"
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

  jmp_buf vm_end_jb;
};

extern VM gvm;  /* global VM */

void vm_open(uint8_t *, int);
void vm_force_exit();
int vm_run(void);
int vm_exec(void);
void stack_dump(char *);

static inline VM *curvm() {
  return &gvm;
}

#define PUSH(ob) (*vm->stackptr++ = (ob))
#define POP() (*--vm->stackptr)
#define TOP() (vm->stackptr[-1])
#define SETTOP(ob) (vm->stackptr[-1] = (ob))

#endif
