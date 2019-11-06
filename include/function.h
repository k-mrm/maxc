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
struct MxcObject *println(size_t);
struct MxcObject *string_size(size_t);
struct MxcObject *string_isempty(size_t);
struct MxcObject *int_tofloat(size_t);
struct MxcObject *object_id(size_t);

enum BLTINFN {
    BLTINFN_PRINT,
    BLTINFN_PRINTLN,
    BLTINFN_STRINGSIZE,
    BLTINFN_STRINGISEMPTY,
    BLTINFN_INTTOFLOAT,
    BLTINFN_OBJECTID,
    BLTINFN_ERROR,
};

#endif
