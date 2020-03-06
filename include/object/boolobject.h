#ifndef MXC_BOOLOBJECT_H
#define MXC_BOOLOBJECT_H

#include <stdint.h>
#include "object/object.h"

typedef struct MxcBool {
    OBJECT_HEAD;
    int64_t boolean;
} MxcBool;

MxcBool *bool_logor(MxcBool *, MxcBool *);
MxcBool *bool_logand(MxcBool *, MxcBool *);
MxcBool *bool_not(MxcBool *);

extern MxcBool _mxc_true;
extern MxcBool _mxc_false;

#define MXC_TRUE  ((MxcObject *)&_mxc_true)
#define MXC_FALSE ((MxcObject *)&_mxc_false)

#define mval_true   value_obj(MXC_TRUE)
#define mval_false  value_obj(MXC_FALSE)

MxcString *true_tostring(MxcObject *);
MxcString *false_tostring(MxcObject *);

#endif
