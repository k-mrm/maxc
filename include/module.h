#ifndef MXC_MODULE_H
#define MXC_MODULE_H

#include "ast.h"
#include "type.h"

typedef struct MxcModule {
    char *name;
    NodeVariable **vars;
} MxcModule;

void set_bltin_func_type(NodeVariable *, Type *, ...);
void set_bltin_var_type(NodeVariable *, Type *);

#endif
