#ifndef MXC_INTOBJECT_H
#define MXC_INTOBJECT_H

#include "object.h"

typedef struct IntObject {
    OBJECT_HEAD;
    int64_t inum;
} IntObject;

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

#define IntAdd(l, r) (new_intobject(l->inum + r->inum))
#define IntSub(l, r) (new_intobject(l->inum - r->inum))
#define IntMul(l, r) (new_intobject(l->inum * r->inum))
#define IntDiv(l, r) (new_intobject(l->inum / r->inum))
#define IntXor(l, r) (new_intobject(l->inum ^ r->inum))

#endif
