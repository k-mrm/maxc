#ifndef MAXC_BYTECODE_H
#define MAXC_BYTECODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "opcode.h"

enum BLTINFN;

typedef struct Bytecode {
  uint8_t *code;
  uint16_t len;
  uint16_t reserved;
} Bytecode;

Bytecode *new_bytecode();

void push_0arg(Bytecode *, enum OPCODE);
void replace_int32(size_t, Bytecode *, int32_t);
void push_int8(Bytecode *, int8_t);
void push_int32(Bytecode *, int32_t);
void push8(Bytecode *, enum OPCODE, int8_t);
void push32(Bytecode *, enum OPCODE, int32_t);

#ifdef MXC_DEBUG
void codedump(uint8_t[], size_t *, Vector *);
#endif

#endif
