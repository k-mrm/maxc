#include <stdlib.h>
#include <string.h>
#include "object/object.h"
#include "context.h"
#include "error/error.h"

MContext *new_econtext(uint8_t *code, size_t nlvars, DebugInfo *d, MContext *prev) {
  MContext *c = malloc(sizeof(MContext));
  c->prev = prev;
  c->code = code;
  c->basepc = c->pc = &code[0];
  if(nlvars != 0) {
    c->lvars = malloc(sizeof(MxcValue) * nlvars);
  }
  else {
    c->lvars = NULL;
  }
  c->nlvars = nlvars;
  c->exc = NULL;
  c->err_handling_enabled = 0;
  c->d = d;

  return c;
}

void delete_context(MContext *c) {
  free(c->lvars);
  free(c);
}
