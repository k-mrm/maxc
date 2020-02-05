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

Bytecode *New_Bytecode();

void push_0arg(Bytecode *, enum OPCODE);
void push_ipush(Bytecode *, int32_t);
void push_cpush(Bytecode *, char);
void push_jmpneq(Bytecode *, size_t);
void push_jmp(Bytecode *, size_t);
void push_jmp_nerr(Bytecode *, int);
void push_store(Bytecode *, int, bool);
void push_load(Bytecode *, int, bool);
void push_strset(Bytecode *, int);
void push_list_set(Bytecode *, int);
void push_fpush(Bytecode *, int);
void push_lpush(Bytecode *, int);
void push_functionset(Bytecode *, int);
void push_bltinfn_set(Bytecode *, enum BLTINFN);
void push_structset(Bytecode *, int);
void push_bltinfn_call(Bytecode *, int);
void push_member_load(Bytecode *, int);
void push_member_store(Bytecode *, int);
void push_iter_next(Bytecode *, int);
void replace_int32(size_t, Bytecode *, int32_t);
void push_int8(Bytecode *, int8_t);
void push_int32(Bytecode *, int32_t);

Vector *set_label_opcode(Bytecode *);

#ifdef MXC_DEBUG
void codedump(uint8_t[], size_t *, Vector *);
#endif

#endif
