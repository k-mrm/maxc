#ifndef MAXC_TYPE_H
#define MAXC_TYPE_H

#include "maxc.h"
#include "struct.h"
#include "util.h"
#include "operator.h"

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

typedef struct Type Type;

typedef char *(*type_to_s)(Type *);

struct Type {
    enum CTYPE type;
    enum TypeImpl impl;
    type_to_s tostring;
    bool optional;
    bool isprimitive;
    MxcOperator *defop;

    union {
        /* list */
        struct {
            Type *ptr;
        };
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
Type *New_Type_Function(Vector *, Type *);
Type *New_Type_With_Ptr(Type *);
Type *New_Type_Unsolved(char *);
Type *New_Type_Variable(char *);
Type *New_Type_With_Struct(MxcStruct);
bool same_type(Type *, Type *);
Type *instantiate(Type *);
bool type_is(Type *, enum CTYPE);
bool is_iterable(Type *);

/* type to string */
char *nonety_tostring(Type *);
char *boolty_tostring(Type *); 
char *intty_tostring(Type *); 
char *floatty_tostring(Type *); 
char *stringty_tostring(Type *);
char *anyty_tostring(Type *);
char *any_varargty_tostring(Type *);
char *functy_tostring(Type *);
char *listty_tostring(Type *);
char *unsolvety_tostring(Type *);
char *structty_tostring(Type *);
char *uninferty_tostring(Type *);

MxcOptional *New_MxcOptional(Type *);

#define mxcty_none (&TypeNone)
#define mxcty_bool (&TypeBool)
#define mxcty_int (&TypeInt)
#define mxcty_float (&TypeFloat)
#define mxcty_string (&TypeString)
#define mxcty_any (&TypeAny)
#define mxcty_any_vararg (&TypeAnyVararg)

extern Type TypeNone;
extern Type TypeBool;
extern Type TypeInt;
extern Type TypeFloat;
extern Type TypeString;
extern Type TypeAny;
extern Type TypeAnyVararg;

#endif
