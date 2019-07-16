#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "env.h"
#include "maxc.h"

class MxcObject;

struct userfunction {
    size_t codelength;
    uint8_t *code;
    size_t nlvars;
};

void new_userfunction(userfunction &, bytecode, Varlist);

typedef MxcObject *(*bltinfn_ty)(MxcObject **, size_t);

MxcObject *print(MxcObject **, size_t);
MxcObject *print_int(MxcObject **, size_t);
MxcObject *print_float(MxcObject **, size_t);
MxcObject *print_bool(MxcObject **, size_t);
MxcObject *print_char(MxcObject **, size_t);
MxcObject *print_string(MxcObject **, size_t);
MxcObject *print_list(MxcObject **, size_t);
MxcObject *println(MxcObject **, size_t);
MxcObject *println_int(MxcObject **, size_t);
MxcObject *println_float(MxcObject **, size_t);
MxcObject *println_bool(MxcObject **, size_t);
MxcObject *println_char(MxcObject **, size_t);
MxcObject *println_string(MxcObject **, size_t);
MxcObject *println_list(MxcObject **, size_t);

enum class BltinFnKind {
    Print,
    PrintInt,
    PrintFloat,
    PrintBool,
    PrintChar,
    PrintString,
    PrintList,
    Println,
    PrintlnInt,
    PrintlnFloat,
    PrintlnBool,
    PrintlnChar,
    PrintlnString,
    PrintlnList,
    StringSize,
};

#endif
