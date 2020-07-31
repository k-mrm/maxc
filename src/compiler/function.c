#include <stdlib.h>
#include "function.h"
#include "bytecode.h"

userfunction *new_userfunction(Bytecode *c, Vector *v, char *name) {
  userfunction *u = malloc(sizeof(userfunction));
  u->code = c->code;
  u->codesize = c->len;
  u->nlvars = v->len;
  u->var_info = v;
  u->name = name;

  return u;
}

