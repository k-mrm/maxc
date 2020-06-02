#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "util.h"

typedef struct userfunction {
  uint16_t codesize;
  uint16_t nlvars;
  uint8_t *code;
  Vector *var_info;
  char *name;
} userfunction;

userfunction *new_userfunction(Bytecode *, Vector *, char *);

#endif
