#ifndef MXC_OBJECT_NUM_H
#define MXC_OBJECT_NUM_H

#include "object/object.h"
#include "object/integerobject.h"
#include "object/intobject.h"

MxcValue num_add(MxcValue, MxcValue);
MxcValue num_sub(MxcValue, MxcValue);
MxcValue num_mul(MxcValue, MxcValue);
MxcValue num_div(MxcValue, MxcValue);
MxcValue num_mod(MxcValue, MxcValue);
MxcValue num_neg(MxcValue);
MxcValue num_eq(MxcValue, MxcValue);
MxcValue num_noteq(MxcValue, MxcValue);

#endif
