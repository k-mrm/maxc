#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "util.h"

typedef struct DebugInfo DebugInfo;
struct DebugInfo {
  Vector *var_info;
  const char *filename;
  char *name;
  Vector *pc_line_map;
};

DebugInfo *new_debuginfo(const char *filename, char *name);
int curlineno(DebugInfo *d, uint8_t *pc, uint8_t *base);

#endif
