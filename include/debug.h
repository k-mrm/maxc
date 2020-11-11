#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "util.h"

typedef struct DebugInfo DebugInfo;
struct DebugInfo {
  Vector *var_info;
  char *filename;
  char *name;
  Vector *pc_line_map;
};

DebugInfo *new_debuginfo(char *filename, char *name);

#endif
