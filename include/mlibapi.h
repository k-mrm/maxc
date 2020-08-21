#ifndef MXC_MLIBAPI_H
#define MXC_MLIBAPI_H

#include "ast.h"
#include "type.h"
#include "context.h"
#include "object/object.h"
#include "object/funcobject.h"

typedef struct MxcCBltin MxcCBltin;

typedef struct MxcModule {
  char *name;
  Vector *cimpl;
  Vector *cmeth;
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

int mlib_parse_arg(MxcValue *arg, int narg, ...);

#endif
