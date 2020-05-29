#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include <stdbool.h>

#include "type.h"
#include "util.h"

enum VARATTR {
  VARATTR_CONST = 0b0001,
  VARATTR_UNINIT = 0b0010,
};

typedef struct Scope {
  Scope *parent;
  Vector *vars;
  Vector *userdef_type;
  bool sblock;
} Scope;

Scope *make_scope(Scope *)
Scope *scope_escape(Scope *);
int chk_var_conflict(Scope *, NodeVariable *);

#define scope_inglobal(scope) (!scope->parent)

#endif
