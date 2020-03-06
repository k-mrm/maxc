#ifndef MXC_INTOBJECT_H
#define MXC_INTOBJECT_H

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    int64_t inum;
} MxcInteger;

struct MxcBool;
typedef struct MxcBool MxcBool;

MxcValue int_add(MxcValue, MxcValue);
MxcValue int_sub(MxcValue, MxcValue);
MxcValue int_mul(MxcValue, MxcValue);
MxcValue int_div(MxcValue, MxcValue);
MxcValue int_mod(MxcValue, MxcValue);
MxcValue int_eq(MxcValue, MxcValue);
MxcValue int_noteq(MxcValue, MxcValue);
MxcValue int_lt(MxcValue, MxcValue);
MxcValue int_lte(MxcValue, MxcValue);
MxcValue int_gt(MxcValue, MxcValue);
MxcValue int_gte(MxcValue, MxcValue);
MxcValue int_inc(MxcValue);
MxcValue int_dec(MxcValue);

#define IntAdd(l, r) (mval_int((l).num + (r).num))
#define IntSub(l, r) (mval_int((l).num - (r).num))
#define IntMul(l, r) (mval_int((l).num * (r).num))
#define IntDiv(l, r) (mval_int((l).num / (r).num))
#define IntXor(l, r) (mval_int((l).num ^ (r).num))

MxcString *int_tostring(MxcObject *);

#endif
