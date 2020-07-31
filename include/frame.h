#ifndef MXC_FRAME_H
#define MXC_FRAME_H

#include <stdio.h>

#include "bytecode.h"
#include "function.h"
#include "error/runtime-err.h"

struct MxcValue;
typedef struct MxcValue MxcValue;
struct MFiber;

typedef struct MContext MContext;
struct MContext {
  MContext *prev;
  char *func_name;
  char *filename;
  uint8_t *code;
  size_t codesize;
  Vector *lvar_info;
  MxcValue *lvars;
  size_t nlvars;
  uint8_t *pc;
  size_t lineno;
  RuntimeErr occurred_rterr;

  struct MFiber *fiber;
};

MContext *new_econtext(Bytecode *, size_t, char *, MContext *);
void delete_frame(MContext *);

#endif
