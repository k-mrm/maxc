#include "operator.h"
#include "error.h"

Vector *mxc_operators;

void New_Op(
        enum MXC_OPERATOR k,
        int op,
        Type *o1,
        Type *o2,
        Type *ret,
        struct NodeFunction *impl
    ) {
    MxcOp *self = malloc(sizeof(MxcOp));

    self->kind = k;
    self->op = op;
    self->operand1 = o1;
    self->operand2 = o2;
    self->ret = ret;
    self->impl = impl;
    self->call = NULL;

    vec_push(mxc_operators, self);
}

void define_operator() {
    MxcOp defs[] = {
        {OPE_BINARY, BIN_ADD,   mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_ADD,   mxcty_float,    mxcty_float,    mxcty_float,    NULL, NULL},
        {OPE_BINARY, BIN_ADD,   mxcty_string,   mxcty_string,   mxcty_string,   NULL, NULL},
        {OPE_BINARY, BIN_SUB,   mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_SUB,   mxcty_float,    mxcty_float,    mxcty_float,    NULL, NULL},
        {OPE_BINARY, BIN_MUL,   mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_MUL,   mxcty_float,    mxcty_float,    mxcty_float,    NULL, NULL},
        {OPE_BINARY, BIN_DIV,   mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_DIV,   mxcty_float,    mxcty_float,    mxcty_float,    NULL, NULL},
        {OPE_BINARY, BIN_MOD,   mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_EQ,    mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_EQ,    mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_NEQ,   mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_NEQ,   mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LT,    mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LT,    mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_GT,    mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_GT,    mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LTE,   mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LTE,   mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_GTE,   mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_GTE,   mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LAND,  mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LAND,  mxcty_bool,     mxcty_bool,     mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LOR,   mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LOR,   mxcty_bool,     mxcty_bool,     mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_LSHIFT,mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
        {OPE_BINARY, BIN_RSHIFT,mxcty_int,      mxcty_int,      mxcty_int,      NULL, NULL},
    };

    int def_len = sizeof(defs) / sizeof(defs[0]);

    mxc_operators = New_Vector_With_Size(def_len);

    for(int i = 0; i < def_len; ++i) {
        MxcOp *a = malloc(sizeof(MxcOp));
        *a = defs[i];
        mxc_operators->data[i] = a;
    }
}

MxcOp *check_op_definition(enum MXC_OPERATOR kind, int op, Type *left, Type *right) {
    for(int i = 0; i < mxc_operators->len; ++i) {
        MxcOp *cur_def = (MxcOp *)mxc_operators->data[i];

        if(kind != cur_def->kind) {
            continue;
        }
        if(op != cur_def->op) {
            continue;
        }
        if(!same_type(left, cur_def->operand1)) {
            continue;
        }
        if(!same_type(right, cur_def->operand2)) {
            continue;
        }

        return cur_def;
    }
    return NULL;
}

enum BINOP op_char1(char c) {
    switch(c) {
    case '+': return BIN_ADD;
    case '-': return BIN_SUB;
    case '*': return BIN_MUL;
    case '/': return BIN_DIV;
    case '%': return BIN_MOD;
    case '<': return BIN_LT;
    case '>': return BIN_GT;
    default : return -1;
    }
}

enum BINOP op_char2(char c1, char c2) {
    switch(c1) {
    case '=':
        switch(c2) {
        case '=':
            return BIN_EQ;
        }
    case '<':
        switch(c2) {
        case '=':
            return BIN_LTE;
        case '<':
            return BIN_LSHIFT;
        }
    case '>':
        switch(c2) {
        case '=':
            return BIN_GTE;
        case '>':
            return BIN_RSHIFT;
        }
    case '!':
        switch(c2) {
        case '=':
            return BIN_NEQ;
        }
    default:
        return -1;
    }

}

char *operator_dump(enum BINOP n) {
    switch(n) {
    case BIN_ADD:   return "+";
    case BIN_SUB:   return "-";
    case BIN_MUL:   return "*";
    case BIN_DIV:   return "/";
    case BIN_MOD:   return "%";
    case BIN_EQ:    return "==";
    case BIN_NEQ:   return "!=";
    case BIN_LT:    return "<";
    case BIN_LTE:   return "<=";
    case BIN_GT:    return ">";
    case BIN_GTE:   return ">=";
    case BIN_LAND:  return "&&";
    case BIN_LOR:   return "||";
    case BIN_LSHIFT:return "<<";
    case BIN_RSHIFT:return ">>";
    default:        return "error!";
    }
}
