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
