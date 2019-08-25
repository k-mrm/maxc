#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "env.h"
#include "maxc.h"

struct MxcObject;

typedef struct userfunction {
    uint16_t codesize;
    uint16_t nlvars;
    uint8_t *code;
} userfunction;

userfunction *New_Userfunction(Bytecode *, Varlist *);

typedef struct MxcObject *(*bltinfn_ty)(size_t);

struct MxcObject *print(size_t);
struct MxcObject *print_int(size_t);
struct MxcObject *print_float(size_t);
struct MxcObject *print_bool(size_t);
struct MxcObject *print_char(size_t);
struct MxcObject *print_string(size_t);
struct MxcObject *print_list(size_t);
struct MxcObject *println(size_t);
struct MxcObject *println_int(size_t);
struct MxcObject *println_float(size_t);
struct MxcObject *println_bool(size_t);
struct MxcObject *println_char(size_t);
struct MxcObject *println_string(size_t);
struct MxcObject *println_list(size_t);
struct MxcObject *string_size(size_t);
struct MxcObject *string_isempty(size_t);
struct MxcObject *int_tofloat(size_t);
struct MxcObject *object_id(size_t);

enum BLTINFN {
    BLTINFN_PRINT,
    BLTINFN_PRINTINT,
    BLTINFN_PRINTFLOAT,
    BLTINFN_PRINTBOOL,
    BLTINFN_PRINTCHAR,
    BLTINFN_PRINTSTRING,
    BLTINFN_PRINTLIST,
    BLTINFN_PRINTLN,
    BLTINFN_PRINTLNINT,
    BLTINFN_PRINTLNFLOAT,
    BLTINFN_PRINTLNBOOL,
    BLTINFN_PRINTLNCHAR,
    BLTINFN_PRINTLNSTRING,
    BLTINFN_PRINTLNLIST,
    BLTINFN_STRINGSIZE,
    BLTINFN_STRINGISEMPTY,
    BLTINFN_INTTOFLOAT,
    BLTINFN_OBJECTID
};

#endif
