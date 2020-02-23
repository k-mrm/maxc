#ifndef MXC_INTOBJECT_H
#define MXC_INTOBJECT_H

#include "object/object.h"

typedef struct MxcInteger {
    OBJECT_HEAD;
    int64_t inum;
} MxcInteger;

struct MxcBool;
typedef struct MxcBool MxcBool;

MxcInteger *new_int(int64_t);

MxcInteger *int_add(MxcInteger *, MxcInteger *);
MxcInteger *int_sub(MxcInteger *, MxcInteger *);
MxcInteger *int_mul(MxcInteger *, MxcInteger *);
MxcInteger *int_div(MxcInteger *, MxcInteger *);
MxcInteger *int_mod(MxcInteger *, MxcInteger *);
MxcBool *int_eq(MxcInteger *, MxcInteger *);
MxcBool *int_noteq(MxcInteger *, MxcInteger *);
MxcBool *int_lt(MxcInteger *, MxcInteger *);
MxcBool *int_lte(MxcInteger *, MxcInteger *);
MxcBool *int_gt(MxcInteger *, MxcInteger *);
MxcBool *int_gte(MxcInteger *, MxcInteger *);
MxcInteger *int_inc(MxcInteger *);
MxcInteger *int_dec(MxcInteger *);

#define IntAdd(l, r) (new_int(l->inum + r->inum))
#define IntSub(l, r) (new_int(l->inum - r->inum))
#define IntMul(l, r) (new_int(l->inum * r->inum))
#define IntDiv(l, r) (new_int(l->inum / r->inum))
#define IntXor(l, r) (new_int(l->inum ^ r->inum))

MxcString *int_tostring(MxcObject *);

#endif
