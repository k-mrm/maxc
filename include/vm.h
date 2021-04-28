#ifndef MAXC_VM_H
#define MAXC_VM_H

#include <setjmp.h>
#include "maxc.h"
#include "context.h"
#include "util.h"
#include "literalpool.h"

// #define DIRECT_THREADED

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
void stackdump(char *);

static inline VM *curvm() {
  return &gvm;
}

#define PUSH(ob) (*vm->stackptr++ = (ob))
#define POP() (*--vm->stackptr)
#define TOP() (vm->stackptr[-1])
#define SETTOP(ob) (vm->stackptr[-1] = (ob))

extern MxcValue screg_a;
extern MxcValue screg_b;

enum stack_cache_state {
  SCXX = 0,
  SCAX = 1,
  SCBX = 2,
  SCBA = 3,
  SCAB = 4,
};

extern enum stack_cache_state scstate;

#define SC_NCACHE() ((scstate + 1) / 2)
#define SC_TOPA()   (scstate % 2)
#define SC_TOPB()   (!(scstate % 2))

#define CLEARCACHE()  \
  do {  \
    switch(scstate) { \
      case SCXX:  break;  \
      case SCAX:  PUSH(screg_a); break; \
      case SCBX:  PUSH(screg_b); break; \
      case SCBA:  PUSH(screg_b); PUSH(screg_a); break;  \
      case SCAB:  PUSH(screg_a); PUSH(screg_b); break;  \
    } \
    scstate = SCXX; \
  } while(0)

#endif
