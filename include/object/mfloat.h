#ifndef MXC_FLOATOBJECT_H
#define MXC_FLOATOBJECT_H

#include "object/object.h"

struct MxcBool;
typedef struct MxcBool MxcBool;

typedef struct MxcFloat {
  OBJECT_HEAD;
  double fnum;
} MxcFloat;

MxcValue float_eq(MxcValue, MxcValue);
MxcValue float_neq(MxcValue, MxcValue);
MxcValue float_lt(MxcValue, MxcValue);
MxcValue float_gt(MxcValue, MxcValue);
MxcValue float_div(MxcValue, MxcValue);

#define FloatAdd(l, r) (mval_float(V2F(l) + V2F(r)))
#define FloatSub(l, r) (mval_float(V2F(l) - V2F(r)))
#define FloatMul(l, r) (mval_float(V2F(l) * V2F(r)))
#define FloatDiv(l, r) (mval_float(V2F(l) / V2F(r)))

MxcValue float_tostring(MxcValue);

#endif
