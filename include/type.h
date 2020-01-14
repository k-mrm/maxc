#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include "maxc.h"
#include "struct.h"
#include "util.h"

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
    CTYPE_ITERATOR,
    CTYPE_OPTIONAL,
    CTYPE_ERROR,
    /* type variable */
    CTYPE_VARIABLE,
};

enum TypeImpl {
    TIMPL_SHOW = 1 << 0,
    TIMPL_ITERABLE = 1 << 1,
};

typedef struct TypeInfo TypeInfo;
typedef struct Type Type;

struct TypeInfo {
    char *name;
    enum TypeImpl impl;
};

struct Type {
    enum CTYPE type;
    enum TypeImpl impl;
    TypeInfo *info;
    bool optional;
    /* list */
    Type *ptr;

    union {
        /* tuple */
        struct {
            Vector *tuple;
        };
        /* function */
        struct {
            Vector *fnarg;
            Type *fnret;
        };
        /* struct */
        struct {
            MxcStruct strct;
            char *name;
        };
        /* error */
        struct {
            char *err_msg;
        };
        /* type variable */
        struct {
            int id; 
            char *type_name;
            Type *instance;
        };
    };
};

typedef struct MxcOptional {
    Type parent;
    Type *base;
    Type *err;
} MxcOptional;

Type *New_Type(enum CTYPE);
Type *New_Type_With_Ptr(Type *);
Type *New_Type_Unsolved(char *);
Type *New_Type_Variable(char *);
Type *New_Type_With_Struct(MxcStruct);
const char *typedump(Type *);
bool same_type(Type *, Type *);
Type *instantiate(Type *);
void type_init();
bool type_is(Type *, enum CTYPE);
bool is_iterable(Type *);

MxcOptional *New_MxcOptional(Type *);

extern Type *mxcty_none;
extern Type *mxcty_bool;
extern Type *mxcty_string;
extern Type *mxcty_int;
extern Type *mxcty_float;
extern Type *mxcty_any;
extern Type *mxcty_any_vararg;

#endif
