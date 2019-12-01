#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "env.h"
#include "error.h"
#include "function.h"
#include "maxc.h"

#define OBJECT_HEAD MxcObject base

typedef struct MxcObject {
    int refcount;
} MxcObject;

typedef struct IntObject {
    OBJECT_HEAD;
    int64_t inum;
} IntObject;

typedef struct FloatObject {
    OBJECT_HEAD;
    double fnum;
} FloatObject;

typedef struct BoolObject {
    OBJECT_HEAD;
    int64_t boolean;
} BoolObject;

typedef struct CharObject {
    OBJECT_HEAD;
    char ch;
} CharObject;

typedef struct ListObject {
    OBJECT_HEAD;
    MxcObject **elem;
    size_t size;
} ListObject;

typedef struct StringObject {
    OBJECT_HEAD;
    const char *str;
} StringObject;

typedef struct ErrorObject {
    OBJECT_HEAD;
    const char *errmsg;
} ErrorObject;

typedef struct TupleObject {
    OBJECT_HEAD;
} TupleObject; // TODO

typedef struct FunctionObject {
    OBJECT_HEAD;
    userfunction *func;
} FunctionObject;

typedef struct BltinFuncObject {
    OBJECT_HEAD;
    bltinfn_ty func;
} BltinFuncObject;

typedef struct StructObject {
    OBJECT_HEAD;
    MxcObject **field;
} StructObject;

typedef struct NullObject {
    OBJECT_HEAD;
} NullObject;

IntObject *new_intobject(int64_t);
IntObject *int_add(IntObject *, IntObject *);
IntObject *int_sub(IntObject *, IntObject *);
IntObject *int_mul(IntObject *, IntObject *);
IntObject *int_div(IntObject *, IntObject *);
IntObject *int_mod(IntObject *, IntObject *);
BoolObject *int_eq(IntObject *, IntObject *);
BoolObject *int_noteq(IntObject *, IntObject *);
BoolObject *int_lt(IntObject *, IntObject *);
BoolObject *int_lte(IntObject *, IntObject *);
BoolObject *int_gt(IntObject *, IntObject *);
BoolObject *int_gte(IntObject *, IntObject *);
BoolObject *float_lt(FloatObject *, FloatObject *);
BoolObject *float_gt(FloatObject *, FloatObject *);
IntObject *int_inc(IntObject *);
IntObject *int_dec(IntObject *);

BoolObject *bool_logor(BoolObject *, BoolObject *);
BoolObject *bool_logand(BoolObject *, BoolObject *);

FloatObject *new_floatobject(double);

CharObject *new_charobject(char);
StringObject *new_stringobject(const char *);
StringObject *str_concat(StringObject *, StringObject *);
FunctionObject *new_functionobject(userfunction *);
BltinFuncObject *new_bltinfnobject(bltinfn_ty);
ListObject *new_listobject(size_t);
StructObject *new_structobject(int);
ErrorObject *new_errorobject(const char *);

extern NullObject MxcNull;
extern BoolObject MxcTrue;
extern BoolObject MxcFalse;

#define Mxc_NULL ((MxcObject *)&MxcNull)
#define Mxc_TRUE ((MxcObject *)&MxcTrue)
#define Mxc_FALSE ((MxcObject *)&MxcFalse)

#define Mxc_RetNull() return INCREF(&MxcNull), Mxc_NULL
#define Mxc_RetTrue() return INCREF(&MxcTrue), Mxc_TRUE
#define Mxc_RetFalse() return INCREF(&MxcFalse), Mxc_FALSE

#define MxcBool_RetTrue() return INCREF(&MxcTrue), &MxcTrue
#define MxcBool_RetFalse() return INCREF(&MxcFalse), &MxcFalse

// test
#define IntAdd(l, r) (new_intobject(l->inum + r->inum))
#define IntSub(l, r) (new_intobject(l->inum - r->inum))
#define IntMul(l, r) (new_intobject(l->inum * r->inum))
#define IntDiv(l, r) (new_intobject(l->inum / r->inum))
#define FloatAdd(l, r) (new_floatobject(l->fnum + r->fnum))
#define FloatSub(l, r) (new_floatobject(l->fnum - r->fnum))
#define FloatMul(l, r) (new_floatobject(l->fnum * r->fnum))
#define FloatDiv(l, r) (new_floatobject(l->fnum / r->fnum))

#endif
