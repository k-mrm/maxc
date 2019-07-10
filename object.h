#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "maxc.h"
#include "bltinfn.h"

struct MxcObject {
    int refcount;
};

struct IntObject : MxcObject {
    int inum32;
};

struct BoolObject : MxcObject {
    bool boolean;
};

struct CharObject : MxcObject {
    char ch;
};

struct ListObject : MxcObject {
    MxcObject **elem;
    size_t allocated;
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

MxcObject *Mxc_malloc(size_t);

IntObject *alloc_intobject(int);
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
IntObject *int_inc(IntObject *);
IntObject *int_dec(IntObject *);

BoolObject *bool_logor(BoolObject *, BoolObject *);
BoolObject *bool_logand(BoolObject *, BoolObject *);

BoolObject *alloc_boolobject(bool);
CharObject *alloc_charobject(char);
StringObject *alloc_stringobject(const char *);
FunctionObject *alloc_functionobject(userfunction);
BltinFuncObject *alloc_bltinfnobject(bltinfn_ty &);
ListObject *alloc_listobject(size_t);

BoolObject *bool_from_int(IntObject *);

}; // namespace Object

#endif
