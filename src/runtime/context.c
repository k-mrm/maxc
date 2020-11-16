#include <stdlib.h>
#include <string.h>
#include "object/object.h"
#include "context.h"
#include "error/error.h"

MContext *new_econtext(uint8_t *code, size_t nlvars, DebugInfo *d, MContext *prev) {
  MContext *f = malloc(sizeof(MContext));
  f->prev = prev;
  f->code = code;
  f->basepc = f->pc = &code[0];
  // f->lvar_info = u->var_info;
  f->lvars = malloc(sizeof(MxcValue) * nlvars);
  for(int i = 0; i < nlvars; ++i)
    f->lvars[i] = mval_invalid;
  f->nlvars = nlvars;
  f->exc = NULL;
  f->err_handling_enabled = 0;
  f->d = d;

  return f;
}

void delete_context(MContext *c) {
  free(c->lvars);
  free(c);
}
