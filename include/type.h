#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include "maxc.h"
#include "struct.h"

enum CTYPE {
    CTYPE_NONE,
    CTYPE_INT,
    CTYPE_UINT,
    CTYPE_INT64,
    CTYPE_UINT64,
    CTYPE_DOUBLE,
    CTYPE_BOOL,
    CTYPE_CHAR,
    CTYPE_STRING,
    CTYPE_LIST,
    CTYPE_TUPLE,
    CTYPE_FUNCTION,
    CTYPE_UNINFERRED,
    CTYPE_ANY_VARARG,
    CTYPE_ANY,
    CTYPE_UNDEFINED,
    CTYPE_STRUCT,
    CTYPE_OPTIONAL,
    CTYPE_ERROR,
};

enum TypeImpl {
    TIMPL_SHOW = 0b1000,
};

typedef struct Type {
    enum CTYPE type;

    struct Type *ptr; // list

    struct Vector *tuple;

    struct Vector *fnarg; // function arg
    struct Type *fnret;   // function rettype

    MxcStruct strct;

    char *name; // struct

    /*
     *  optional type
     */
    bool optional;

    /*
     *  Error
     */
    char *err_msg;

    unsigned int impl;
} Type;

typedef struct MxcOptional {
    Type parent;
    Type *base;
    Type *err;
} MxcOptional;

Type *New_Type(enum CTYPE);
Type *New_Type_With_Ptr(Type *);
Type *New_Type_With_Str(char *);
Type *New_Type_With_Struct(MxcStruct);
const char *typedump(Type *);
bool same_type(Type *, Type *);
void type_init();
bool type_is(Type *, enum CTYPE);

MxcOptional *New_MxcOptional(Type *base);

extern Type *mxcty_none;
extern Type *mxcty_bool;
extern Type *mxcty_string;
extern Type *mxcty_int;
extern Type *mxcty_float;

#endif
