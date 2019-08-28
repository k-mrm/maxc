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
    Type() {}
    Type(CTYPE ty) : type(ty) {}
    Type(CTYPE ty, int size) : type(ty, size) {} //?
    Type(Type *p) : type(CTYPE::LIST), ptr(p) {} // list
    Type(Type_v a, Type *r) :
        type(CTYPE::FUNCTION), fnarg(a), fnret(r) {} // function
    Type(std::string &n): type(CTYPE::UNDEFINED), name(n) {}
    Type(MxcStruct &s): type(CTYPE::STRUCT), strct(s) {}    //struct
    */
} Type;

Type *New_Type(enum CTYPE);
Type *New_Type_With_Ptr(Type *);
Type *New_Type_With_Str(char *);
Type *New_Type_With_Struct(MxcStruct);
const char *typedump(Type *);
void type_init();
bool type_is(Type *, enum CTYPE);

extern Type *mxcty_none;
extern Type *mxcty_bool;
extern Type *mxcty_string;
extern Type *mxcty_int;
extern Type *mxcty_float;

#endif
