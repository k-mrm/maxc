#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include <stdio.h>
#include "maxc.h"
#include "debug.h"
#include "bytecode.h"
#include "function.h"
#include "object/object.h"
#include "object/mexception.h"

struct MFiber;
typedef struct MContext MContext;
struct MContext {
  MContext *prev;
  char *filename;
  mptr_t *code;
  size_t codesize;
  Vector *lvar_info;
  MxcValue *lvars;
  size_t nlvars;
  mptr_t *pc;
  mptr_t *basepc;
  MException *exc;
  int err_handling_enabled;

  struct MFiber *fiber;

  DebugInfo *d;
};

MContext *new_econtext(mptr_t *, size_t, DebugInfo *, MContext *);
void delete_context(MContext *);

#endif
