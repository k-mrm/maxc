#include <stdlib.h>
#include <string.h>

#include "object/object.h"
#include "frame.h"
#include "error/error.h"

MContext *new_econtext(uint8_t *code, size_t nlvars, char *name, MContext *prev) {
  MContext *f = malloc(sizeof(MContext));
  f->prev = prev;
  f->func_name = name;
  f->code = code;
  f->pc = &code[0];
  // f->lvar_info = u->var_info;
  f->lvars = malloc(sizeof(MxcValue) * nlvars);
  for(int i = 0; i < nlvars; ++i)
    f->lvars[i] = mval_invalid;
  f->nlvars = nlvars;

  return f;
}

void delete_frame(MContext *c) {
  free(c->lvars);
  free(c);
}
