#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include <stdbool.h>

#include "type.h"
#include "util.h"

enum VARATTR {
  VARATTR_CONST = 0b0001,
  VARATTR_UNINIT = 0b0010,
};

enum scopetype {
  BLOCKSCOPE,
  FUNCSCOPE,
};

typedef struct Scope Scope;
struct Scope {
  Scope *parent;
  Vector *vars;
  Vector *userdef_type;
  Vector *fscope_vars;
  enum scopetype type;
  bool fscope_gbl;
  int err;
  int ngvar;
};

Scope *make_scope(Scope *, enum scopetype);
Scope *scope_escape(Scope *);
void scope_push_var(Scope *, NodeVariable *);
size_t var_assign_id(Scope *);
int chk_var_conflict(Scope *, NodeVariable *);

#define scope_isglobal(scope) (!scope->parent)
#define fscope_isglobal(scope) (scope->fscope_gbl)

#endif
