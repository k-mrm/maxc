#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include <stdbool.h>

#include "type.h"
#include "util.h"

struct NodeVariable;
typedef struct NodeVariable NodeVariable;
enum BLTINFN;

enum VARATTR {
  VARATTR_CONST = 0b0001,
  VARATTR_UNINIT = 0b0010,
};

typedef struct Varlist {
  Vector *vars;
} Varlist;

Varlist *new_varlist(void);
void varlist_push(Varlist *, NodeVariable *);
void varlist_mulpush(Varlist *, Varlist *);
size_t var_set_number(Varlist *);

typedef struct Env {
  Varlist *vars;
  Vector *userdef_type;
  struct Env *parent;
  bool isglb;
} Env;

Env *New_Env(void);
Env *New_Env_Global(void);

typedef struct Scope {
  Env *current;
} Scope;

Env *scope_make(Scope *);
Env *scope_escape(Scope *);
bool scope_isglobal(Scope);
int chk_var_conflict(Scope, NodeVariable *);

typedef struct FuncEnv {
  Env *current;
} FuncEnv;

Env *funcenv_make(FuncEnv *s);
Env *funcenv_escape(FuncEnv *s);
bool funcenv_isglobal(FuncEnv s);

#endif
