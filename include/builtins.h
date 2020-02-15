#ifndef MXC_BUILTINS_H
#define MXC_BUILTINS_H

#include "env.h"

struct MxcObject;
typedef struct MxcObject MxcObject;
struct Frame;
typedef struct Frame Frame;

extern Varlist *bltin_funcs;

enum BLTINFN {
    BLTINFN_PRINT,
    BLTINFN_PRINTLN,
    BLTINFN_STRINGSIZE,
    BLTINFN_INTTOFLOAT,
    BLTINFN_OBJECTID,
    BLTINFN_ERROR,
    BLTINFN_EXIT,
    BLTINFN_READLINE,
    BLTINFN_LISTLEN,
};

/* impl */
MxcObject *print(MxcObject **, size_t);
MxcObject *println(MxcObject **, size_t);
MxcObject *string_size(MxcObject **, size_t);
MxcObject *int_tofloat(MxcObject **, size_t);
MxcObject *object_id(MxcObject **, size_t);
MxcObject *mxcerror(MxcObject **, size_t);
MxcObject *mxcsys_exit(MxcObject **, size_t);
MxcObject *mxc_readline(MxcObject **, size_t);

#endif
