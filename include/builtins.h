#ifndef MXC_BUILTINS_H
#define MXC_BUILTINS_H

#include "maxc.h"
#include "env.h"

struct MxcObject;
typedef struct MxcObject MxcObject;

extern Varlist *bltin_funcs;

enum BLTINFN {
    BLTINFN_PRINT,
    BLTINFN_PRINTLN,
    BLTINFN_STRINGSIZE,
    BLTINFN_STRINGISEMPTY,
    BLTINFN_INTTOFLOAT,
    BLTINFN_OBJECTID,
    BLTINFN_ERROR,
    BLTINFN_EXIT,
    BLTINFN_READLINE,
};

typedef struct MxcObject *(*bltinfn_ty)(MxcObject **, size_t);

MxcObject *print(MxcObject **, size_t);
MxcObject *println(MxcObject **, size_t);
MxcObject *string_size(MxcObject **, size_t);
MxcObject *string_isempty(MxcObject **, size_t);
MxcObject *int_tofloat(MxcObject **, size_t);
MxcObject *object_id(MxcObject **, size_t);
MxcObject *mxcerror(MxcObject **, size_t);
MxcObject *mxcsys_exit(MxcObject **, size_t);

extern bltinfn_ty bltinfns[];

#endif
