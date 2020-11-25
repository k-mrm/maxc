#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "debug.h"
#include "bytecode.h"
#include "util.h"

typedef struct userfunction {
  uint16_t codesize;
  uint16_t nlvars;
  uint8_t *code;
  DebugInfo *d;
} userfunction;

userfunction *new_userfunction(Bytecode *c, DebugInfo *d, size_t nlvars);

#endif
