#ifndef MAXC_OPERATOR_H
#define MAXC_OPERATOR_H

#include "maxc.h"
#include "type.h"

enum MXC_OPERATOR {
    OPE_BINARY,
    OPE_UNARY
};

enum BINOP {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,
    BIN_EQ,
    BIN_NEQ,
    BIN_LT,
    BIN_LTE,
    BIN_GT,
    BIN_GTE,
    BIN_LAND,
    BIN_LOR,
    BIN_LSHIFT,
    BIN_RSHIFT,
};

enum UNAOP {
    UNA_INC,
    UNA_DEC,
    UNA_PLUS,
    UNA_MINUS,
};

typedef struct {
    enum MXC_OPERATOR kind; 
    int op; 
    Type *operand1;
    Type *operand2;
} MxcOp;

void define_operator();

#endif
