#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include <stdbool.h>

#include "type.h"
#include "util.h"

enum VARATTR {
  VARATTR_CONST = 0b0001,
  VARATTR_UNINIT = 0b0010,
};

typedef struct Scope Scope;
struct Scope {
  Scope *parent;
  Vector *vars;
  Vector *userdef_type;
  Vector *fscope_vars;
  int fblock;
  int fscope_gbl;
};

Scope *make_scope(Scope *, int);
Scope *scope_escape(Scope *);
void scope_push_var(Scope *, NodeVariable *);
int chk_var_conflict(Scope *, NodeVariable *);

#define scope_isglobal(scope) (!scope->parent)
#define fscope_isglobal(scope) (scope->fscope_gbl)

#define func_block  1
#define local_scope 0

#endif
