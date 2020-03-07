#ifndef MXC_MODULE_H
#define MXC_MODULE_H

#include "ast.h"
#include "type.h"
#include "object/object.h"

typedef struct MxcCBltin MxcCBltin;
typedef struct MxcValue (*CFunction)(Frame *, MxcValue *, size_t);

typedef struct MxcModule {
    char *name;
    Vector *cbltins;
} MxcModule;

typedef struct MxcCBltin {
    NodeVariable *var;
    MxcValue impl;
} MxcCBltin;

typedef struct _MxcCMethod {
    NodeVariable *var;
    CFunction meth;
} _MxcCMethod;

void define_cmethod(Vector *, char *, CFunction, Type *, ...);
void define_cconst(Vector *, char *, MxcValue, Type *);
MxcCBltin *new_cbltin(NodeVariable *, MxcValue);
void convert_cmeth(Vector *, _MxcCMethod *);
void cbltin_add_obj(Vector *, NodeVariable *, MxcValue);

/* builtin variable */
void builtin_Init(void);
extern Vector *Global_Cbltins;

#endif
