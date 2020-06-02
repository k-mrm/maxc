#ifndef MXC_MODULE_H
#define MXC_MODULE_H

#include "ast.h"
#include "type.h"
#include "frame.h"
#include "object/object.h"

typedef struct MxcCBltin MxcCBltin;
typedef struct MxcValue (*cfunction)(Frame *, MxcValue *, size_t);

typedef struct MxcModule {
  char *name;
  Vector *cimpl;
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
void define_cconst(MxcModule *, char *, MxcValue, Type *);

/* builtin variable */
void builtin_Init(void);
extern Vector *Global_Cbltins;

#endif
