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
  MxcValue *gvars;
  size_t pc;
  size_t nlvars;
  size_t ngvars;
  size_t lineno;
  MxcValue *stackptr;
  MxcValue *stackbase;
  RuntimeErr occurred_rterr;

  struct MFiber *fiber;
};

MContext *new_global_frame(Bytecode *, int);
MContext *new_frame(userfunction *, MContext *);
void delete_frame(MContext *);

#endif
