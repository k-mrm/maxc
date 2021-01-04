#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "maxc.h"
#include "debug.h"
#include "bytecode.h"

struct cgen {
  struct cgen *prev;
  Bytecode *iseq;
  MxcValue *gvars;
  int ngvars;
  Vector *ltable;
  Vector *loopstack;
  DebugInfo *d;
};

struct cgen *compile(Vector *, int);
struct cgen *compile_repl(Vector *, struct cgen *);

struct cgen *newcgen_glb(int ngvars);
struct cgen *newcgen(struct cgen *p, char *name);

#endif
