#ifndef MXC_MLIBAPI_H
#define MXC_MLIBAPI_H

#include "ast.h"
#include "type.h"
#include "context.h"
#include "object/object.h"
#include "object/mfunc.h"

typedef struct MxcCBltin MxcCBltin;

typedef struct MxcModule {
  char *name;
  Vector *cimpl;
  Vector *cmeth;
  Vector *ctypes;
} MxcModule;

typedef struct _MxcCMethod {
  NodeVariable *var;
  cfunction meth;
} _MxcCMethod;

typedef struct MCimpl {
  NodeVariable *var;
  MxcValue impl;
} MCimpl;

MxcModule *new_mxcmodule(char *);
void define_cfunc(MxcModule *, char *, cfunction, Type *, ...);
void define_cmeth(MxcModule *, char *, cfunction, Type *, ...);
void define_cconst(MxcModule *, char *, MxcValue, Type *);

void define_ctype(MxcModule *, Type *);

int mlib_parse_arg(MxcValue *arg, int narg, ...);

void reg_gmodule(MxcModule *m);

#endif
