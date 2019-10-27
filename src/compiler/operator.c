#include "operator.h"
#include "error.h"

Vector *mxc_operators;

void define_operator() {
    MxcOp defs[] = {
        {OPE_BINARY, BIN_ADD,   mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_ADD,   mxcty_float,    mxcty_float,    mxcty_float},
        {OPE_BINARY, BIN_ADD,   mxcty_string,   mxcty_string,   mxcty_string},
        {OPE_BINARY, BIN_SUB,   mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_SUB,   mxcty_float,    mxcty_float,    mxcty_float},
        {OPE_BINARY, BIN_MUL,   mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_MUL,   mxcty_float,    mxcty_float,    mxcty_float},
        {OPE_BINARY, BIN_DIV,   mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_DIV,   mxcty_float,    mxcty_float,    mxcty_float},
        {OPE_BINARY, BIN_MOD,   mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_EQ,    mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_EQ,    mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_NEQ,   mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_NEQ,   mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_LT,    mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_LT,    mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_GT,    mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_GT,    mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_LTE,   mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_LTE,   mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_GTE,   mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_GTE,   mxcty_float,    mxcty_float,    mxcty_bool},
        {OPE_BINARY, BIN_LAND,  mxcty_int,      mxcty_int,      mxcty_bool},
        {OPE_BINARY, BIN_LAND,  mxcty_bool,     mxcty_bool,     mxcty_bool},
        {OPE_BINARY, BIN_LSHIFT,mxcty_int,      mxcty_int,      mxcty_int},
        {OPE_BINARY, BIN_RSHIFT,mxcty_int,      mxcty_int,      mxcty_int},
    };

    int def_len = sizeof(defs) / sizeof(defs[0]);

    mxc_operators = New_Vector_With_Size(def_len);

    for(int i = 0; i < def_len; ++i) {
        MxcOp *a = malloc(sizeof(MxcOp));
        *a = defs[i];
        mxc_operators->data[i] = a;
    }
}
