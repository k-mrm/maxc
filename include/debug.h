#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "maxc.h"
#include "util.h"

typedef struct DebugInfo DebugInfo;
struct DebugInfo {
  Vector *var_info;
  const char *filename;
  char *name;
  Vector *pc_line_map;
};

DebugInfo *new_debuginfo(const char *filename, char *name);
int curlineno(DebugInfo *d, mptr_t *pc, mptr_t *base);

#endif
