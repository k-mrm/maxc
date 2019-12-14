#ifndef MAXC_BYTECODE_H
#define MAXC_BYTECODE_H

#include "maxc.h"
#include "util.h"

enum BLTINFN;

enum OPCODE {
    OP_END,
    OP_PUSH,
    OP_IPUSH,
    OP_PUSHCONST_0,
    OP_PUSHCONST_1,
    OP_PUSHCONST_2,
    OP_PUSHCONST_3,
    OP_PUSHTRUE,
    OP_PUSHFALSE,
    OP_PUSHNULL,
    OP_FPUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_LOGOR,
    OP_LOGAND,
    OP_EQ,
    OP_NOTEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    // float
    OP_FADD,
    OP_FSUB,
    OP_FMUL,
    OP_FDIV,
    OP_FMOD,
    OP_FLOGOR,
    OP_FLOGAND,
    OP_FEQ,
    OP_FNOTEQ,
    OP_FLT,
    OP_FLTE,
    OP_FGT,
    OP_FGTE,
    // float end
    OP_JMP,
    OP_JMP_EQ,
    OP_JMP_NOTEQ,
    OP_JMP_NOTERR,
    OP_INC,
    OP_DEC,
    OP_LOAD_GLOBAL,
    OP_LOAD_LOCAL,
    OP_STORE_GLOBAL,
    OP_STORE_LOCAL,
    OP_LISTSET,
    OP_LISTLENGTH,
    OP_SUBSCR,
    OP_SUBSCR_STORE,
    OP_STRINGSET,
    OP_TUPLESET,
    OP_FUNCTIONSET,
    OP_BLTINFN_SET,
    OP_STRUCTSET,
    OP_RET,
    OP_CALL,
    OP_CALL_BLTIN,
    OP_MEMBER_LOAD,
    OP_MEMBER_STORE,
    OP_ITER_NEXT,
    OP_STRCAT,
    OP_SHOWINT,
    OP_SHOWFLOAT,
    OP_SHOWBOOL,
};

typedef struct Bytecode {
    uint8_t *code;
    uint16_t len;
    uint16_t reserved;
} Bytecode;

Bytecode *New_Bytecode();

void push_0arg(Bytecode *, enum OPCODE);
void push_ipush(Bytecode *, int32_t);
void push_jmpneq(Bytecode *, size_t);
void push_jmp(Bytecode *, size_t);
void push_jmp_nerr(Bytecode *, int);
void push_store(Bytecode *, int, bool);
void push_load(Bytecode *, int, bool);
void push_strset(Bytecode *, int);
void push_list_set(Bytecode *, int);
void push_fpush(Bytecode *, int);
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

#ifdef MXC_DEBUG
void codedump(uint8_t[], size_t *, Vector *);
#endif

#endif
