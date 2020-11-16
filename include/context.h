#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include <stdio.h>
#include "debug.h"
#include "bytecode.h"
#include "function.h"
#include "object/mexception.h"

struct MxcValue;
typedef struct MxcValue MxcValue;
struct MFiber;

typedef struct MContext MContext;
struct MContext {
  MContext *prev;
  char *filename;
  uint8_t *code;
  size_t codesize;
  Vector *lvar_info;
  MxcValue *lvars;
  size_t nlvars;
  uint8_t *pc;
  MException *exc;
  int err_handling_enabled;

  struct MFiber *fiber;

  DebugInfo *d;
};

MContext *new_econtext(uint8_t *, size_t, DebugInfo *, MContext *);
void delete_context(MContext *);

#endif
