#ifndef MXC_FLOATOBJECT_H
#define MXC_FLOATOBJECT_H

#include "object/object.h"

struct MxcBool;
typedef struct MxcBool MxcBool;

typedef struct MxcFloat {
    OBJECT_HEAD;
    double fnum;
} MxcFloat;

MxcFloat *new_float(double);

MxcBool *float_eq(MxcFloat *, MxcFloat *);
MxcBool *float_neq(MxcFloat *, MxcFloat *);
MxcBool *float_lt(MxcFloat *, MxcFloat *);
MxcBool *float_gt(MxcFloat *, MxcFloat *);
MxcFloat *float_div(MxcFloat *, MxcFloat *);

#define FloatAdd(l, r) (new_float(l->fnum + r->fnum))
#define FloatSub(l, r) (new_float(l->fnum - r->fnum))
#define FloatMul(l, r) (new_float(l->fnum * r->fnum))
#define FloatDiv(l, r) (new_float(l->fnum / r->fnum))

MxcString *float_tostring(MxcObject *);

#endif
