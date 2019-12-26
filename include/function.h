#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "bytecode.h"
#include "env.h"
#include "maxc.h"

struct MxcObject;

typedef struct MxcObject MxcObject;

typedef struct userfunction {
    uint16_t codesize;
    uint16_t nlvars;
    uint8_t *code;
} userfunction;

userfunction *New_Userfunction(Bytecode *, Varlist *);

typedef struct MxcObject *(*bltinfn_ty)(MxcObject ***, size_t);

MxcObject *print(MxcObject **, size_t);
MxcObject *println(MxcObject **, size_t);
MxcObject *string_size(MxcObject **, size_t);
MxcObject *string_isempty(MxcObject **, size_t);
MxcObject *int_tofloat(MxcObject **, size_t);
MxcObject *object_id(MxcObject **, size_t);
MxcObject *mxcerror(MxcObject **, size_t);

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
