#include <stdlib.h>
#include "function.h"
#include "bytecode.h"

userfunction *new_userfunction(Bytecode *c, DebugInfo *d, size_t nlvars) {
  userfunction *u = malloc(sizeof(userfunction));
  u->code = c->code;
  u->codesize = c->len;
  u->nlvars = nlvars;
  u->d = d;

  return u;
}

