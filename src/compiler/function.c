#include <stdlib.h>
#include "function.h"
#include "bytecode.h"

userfunction *new_userfunction(Bytecode *c, DebugInfo *d) {
  userfunction *u = malloc(sizeof(userfunction));
  u->code = c->code;
  u->codesize = c->len;
  u->nlvars = d->var_info->len;
  u->d = d;

  return u;
}

