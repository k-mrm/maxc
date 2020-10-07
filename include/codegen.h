#ifndef MAXC_BYTECODE_GEN_H
#define MAXC_BYTECODE_GEN_H

#include "bytecode.h"
#include "maxc.h"

struct cgen {
  struct cgen *prev;
  Bytecode *iseq;
  MxcValue *gvars;
  int ngvars;
  Vector *ltable;
  Vector *loopstack;
};

struct cgen *compile(MInterp *, Vector *, int);
struct cgen *compile_repl(MInterp *, Vector *, struct cgen *);

struct cgen *newcgen_glb(int ngvars);
struct cgen *newcgen(struct cgen *p);

#endif
