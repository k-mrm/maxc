#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

DebugInfo *new_debuginfo(const char *fname, char *name) {
  DebugInfo *d = malloc(sizeof(DebugInfo));
  d->filename = fname;
  d->name = name;
  d->var_info = new_vector();
  d->pc_line_map = new_vector_capa(64);
  return d;
}

int curlineno(DebugInfo *d, uint8_t *pc, uint8_t *base) {
  int offset = pc - base - 1;
  return (int)(intptr_t)d->pc_line_map->data[offset];
}
