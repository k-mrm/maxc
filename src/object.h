#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "env.h"
#include "error.h"
#include "function.h"
#include "maxc.h"

struct MxcObject {
    int refcount;
};

struct IntObject : MxcObject {
    int64_t inum;
};

struct FloatObject : MxcObject {
    double fnum;
};

struct BoolObject : MxcObject {
    bool boolean;
};

struct CharObject : MxcObject {
    char ch;
};

struct ListObject : MxcObject {
    MxcObject **elem;
    size_t size;
};

struct StringObject : MxcObject {
    const char *str;
};

struct TupleObject : MxcObject {}; // TODO

struct FunctionObject : MxcObject {
    userfunction func;
};

struct BltinFuncObject : MxcObject {
    bltinfn_ty func;
};

struct NullObject : MxcObject {};

namespace Object {

void init();

IntObject *alloc_intobject(int64_t);
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

FloatObject *alloc_floatobject(double);

CharObject *alloc_charobject(char);
StringObject *alloc_stringobject(const char *);
FunctionObject *alloc_functionobject(userfunction);
BltinFuncObject *alloc_bltinfnobject(bltinfn_ty &);
ListObject *alloc_listobject(size_t);
}; // namespace Object

extern NullObject Null;
extern BoolObject MxcTrue;
extern BoolObject MxcFalse;

#define Mxc_RetNull() return INCREF(&Null), &Null
#define Mxc_RetTrue() return INCREF(&MxcTrue), &MxcTrue
#define Mxc_RetFalse() return INCREF(&MxcFalse), &MxcFalse

// test
#define IntAdd(l, r) (Object::alloc_intobject(l->inum + r->inum))
#define IntSub(l, r) (Object::alloc_intobject(l->inum - r->inum))
#define IntMul(l, r) (Object::alloc_intobject(l->inum * r->inum))
#define IntDiv(l, r) (Object::alloc_intobject(l->inum / r->inum))
#define FloatAdd(l, r) (Object::alloc_floatobject(l->fnum + r->fnum))
#define FloatSub(l, r) (Object::alloc_floatobject(l->fnum - r->fnum))
#define FloatMul(l, r) (Object::alloc_floatobject(l->fnum * r->fnum))
#define FloatDiv(l, r) (Object::alloc_floatobject(l->fnum / r->fnum))

#endif
