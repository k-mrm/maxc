#ifndef MXC_FLOATOBJECT_H
#define MXC_FLOATOBJECT_H

#include "object/object.h"

struct BoolObject;
typedef struct BoolObject BoolObject;

typedef struct FloatObject {
    OBJECT_HEAD;
    double fnum;
} FloatObject;

FloatObject *new_floatobject(double);

BoolObject *float_eq(FloatObject *, FloatObject *);
BoolObject *float_neq(FloatObject *, FloatObject *);
BoolObject *float_lt(FloatObject *, FloatObject *);
BoolObject *float_gt(FloatObject *, FloatObject *);
FloatObject *float_div(FloatObject *, FloatObject *);

#define FloatAdd(l, r) (new_floatobject(l->fnum + r->fnum))
#define FloatSub(l, r) (new_floatobject(l->fnum - r->fnum))
#define FloatMul(l, r) (new_floatobject(l->fnum * r->fnum))
#define FloatDiv(l, r) (new_floatobject(l->fnum / r->fnum))

#endif
