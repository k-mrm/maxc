#ifndef MAXC_BYTECODE_H
#define MAXC_BYTECODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "maxc.h"
#include "util.h"
#include "opcode.h"

typedef struct Bytecode {
  mptr_t *code;
  uint16_t len;
  uint16_t reserved;
} Bytecode;

Bytecode *new_bytecode(void);

void op_addr_table_init();
void pushop(Bytecode *self, enum OPCODE);
void pusharg(Bytecode *self, mptr_t);
void replace_int(size_t, Bytecode *, int64_t);

#ifdef MXC_DEBUG
void codedump(mptr_t[], size_t *, Vector *);
#endif

#endif
