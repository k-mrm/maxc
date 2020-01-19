#include "operator.h"
#include "error/error.h"

Vector *mxc_bin_operators;
Vector *mxc_una_operators;

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

    vec_push(k == OPE_BINARY ? mxc_bin_operators : mxc_una_operators,
             self);
}

MxcOperator opdefs_integer[] = {
    /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
    {OPE_BINARY, BIN_ADD,   mxcty_int, mxcty_int,   NULL,   "+"},
    {OPE_BINARY, BIN_SUB,   mxcty_int, mxcty_int,   NULL,   "-"},
    {OPE_BINARY, BIN_MUL,   mxcty_int, mxcty_int,   NULL,   "*"},
    {OPE_BINARY, BIN_DIV,   mxcty_int, mxcty_int,   NULL,   "/"},
    {OPE_BINARY, BIN_MOD,   mxcty_int, mxcty_int,   NULL,   "%"},
    {OPE_BINARY, BIN_EQ,    mxcty_int, mxcty_bool,  NULL,   "=="},
    {OPE_BINARY, BIN_NEQ,   mxcty_int, mxcty_bool,  NULL,   "!="},
    {OPE_BINARY, BIN_LT,    mxcty_int, mxcty_bool,  NULL,   "<"},
    {OPE_BINARY, BIN_LTE,   mxcty_int, mxcty_bool,  NULL,   "<="},
    {OPE_BINARY, BIN_GT,    mxcty_int, mxcty_bool,  NULL,   ">"},
    {OPE_BINARY, BIN_GTE,   mxcty_int, mxcty_bool,  NULL,   ">="},
    {OPE_BINARY, BIN_LAND,  mxcty_int, mxcty_bool,  NULL,   "and"},
    {OPE_BINARY, BIN_LOR,   mxcty_int, mxcty_bool,  NULL,   "or"},
    {OPE_BINARY, BIN_LSHIFT,mxcty_int, mxcty_int,   NULL,   "<<"},
    {OPE_BINARY, BIN_RSHIFT,mxcty_int, mxcty_int,   NULL,   ">>"},
    {OPE_UNARY,  UNA_INC,   NULL,      mxcty_int,   NULL,   "++"},
    {OPE_UNARY,  UNA_DEC,   NULL,      mxcty_int,   NULL,   "--"},
    {OPE_UNARY,  UNA_MINUS, NULL,      mxcty_int,   NULL,   "-"},
};

MxcOperator opdefs_boolean[] = {
    /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
    {OPE_BINARY, BIN_EQ,    mxcty_bool, mxcty_bool, NULL,   "=="},
    {OPE_BINARY, BIN_NEQ,   mxcty_bool, mxcty_bool, NULL,   "!="},
    {OPE_BINARY, BIN_LAND,  mxcty_bool, mxcty_bool, NULL,   "and"},
    {OPE_BINARY, BIN_LOR,   mxcty_bool, mxcty_bool, NULL,   "or"},
};

MxcOperator opdefs_float[] = {
    /* kind */  /* ope */   /* ope2 */ /* ret */    /* fn *//* opname */
    {OPE_BINARY, BIN_ADD,   mxcty_float, mxcty_float, NULL, "+"},
    {OPE_BINARY, BIN_SUB,   mxcty_float, mxcty_float, NULL, "-"},
    {OPE_BINARY, BIN_MUL,   mxcty_float, mxcty_float, NULL, "*"},
    {OPE_BINARY, BIN_DIV,   mxcty_float, mxcty_float, NULL, "/"},
    {OPE_BINARY, BIN_EQ,    mxcty_float, mxcty_bool,  NULL, "=="},
    {OPE_BINARY, BIN_NEQ,   mxcty_float, mxcty_bool,  NULL, "!="},
    {OPE_BINARY, BIN_LT,    mxcty_float, mxcty_bool,  NULL, "<"},
    {OPE_BINARY, BIN_LTE,   mxcty_float, mxcty_bool,  NULL, "<="},
    {OPE_BINARY, BIN_GT,    mxcty_float, mxcty_bool,  NULL, ">"},
    {OPE_BINARY, BIN_GTE,   mxcty_float, mxcty_bool,  NULL, ">="},
    {OPE_UNARY,  UNA_MINUS, NULL,        mxcty_float, NULL, "-"},
};

MxcOperator opdefs_string[] = {
    /* kind */  /* ope */   /* ope2 */    /* ret */    /* fn */ /* opname */
    {OPE_BINARY, BIN_ADD,   mxcty_string, mxcty_string, NULL,   "+"}
};

void define_operator() {
    MxcOp bin_defs[] = {
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
        {OPE_BINARY, BIN_EQ,    mxcty_bool,     mxcty_bool,     mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_EQ,    mxcty_float,    mxcty_float,    mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_NEQ,   mxcty_int,      mxcty_int,      mxcty_bool,     NULL, NULL},
        {OPE_BINARY, BIN_NEQ,   mxcty_bool,     mxcty_bool,     mxcty_bool,     NULL, NULL},
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

    MxcOp una_defs[] = {
        {OPE_UNARY, UNA_INC,    mxcty_int,   NULL, mxcty_int,    NULL, NULL},
        {OPE_UNARY, UNA_DEC,    mxcty_int,   NULL, mxcty_int,    NULL, NULL},
        {OPE_UNARY, UNA_NOT,    mxcty_bool,  NULL, mxcty_bool,   NULL, NULL},
        {OPE_UNARY, UNA_MINUS,  mxcty_int,   NULL, mxcty_int,    NULL, NULL},
        {OPE_UNARY, UNA_MINUS,  mxcty_float, NULL, mxcty_float,  NULL, NULL},
    };

    int bin_def_len = sizeof(bin_defs) / sizeof(bin_defs[0]);
    mxc_bin_operators = New_Vector_With_Size(bin_def_len);

    for(int i = 0; i < bin_def_len; ++i) {
        MxcOp *a = malloc(sizeof(MxcOp));
        *a = bin_defs[i];
        mxc_bin_operators->data[i] = a;
    }

    int una_def_len = sizeof(una_defs) / sizeof(una_defs[0]);
    mxc_una_operators = New_Vector_With_Size(una_def_len);

    for(int i = 0; i < una_def_len; ++i) {
        MxcOp *a = malloc(sizeof(MxcOp));
        *a = una_defs[i];
        mxc_una_operators->data[i] = a;
    }
}

MxcOperator *check_operator() {
    ;
}

MxcOp *check_op_definition(enum MXC_OPERATOR kind, int op, Type *left, Type *right) {
    Vector *operators = (kind == OPE_BINARY ? mxc_bin_operators : mxc_una_operators);

    for(int i = 0; i < operators->len; ++i) {
        MxcOp *cur_def = (MxcOp *)operators->data[i];

        if(op != cur_def->op) {
            continue;
        }
        if(!same_type(left, cur_def->operand1)) {
            continue;
        }
        if(right) {
            if(!same_type(right, cur_def->operand2)) {
                continue;
            }
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

char *operator_dump(enum MXC_OPERATOR k, int n) {
    if(k == OPE_BINARY) {
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
    else if(k == OPE_UNARY) {
        switch(n) {
        case UNA_INC:   return "++";
        case UNA_DEC:   return "--";
        case UNA_PLUS:  return "+";
        case UNA_MINUS: return "-";
        default:        return "error!";
        }
    }

    /* unreachable */
}
