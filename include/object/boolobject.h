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
#define Mxc_RetTrue() return INCREF(&_mxc_true), MXC_TRUE
#define Mxc_RetFalse() return INCREF(&_mxc_false), MXC_FALSE
#define MxcBool_RetTrue() return INCREF(&_mxc_true), &_mxc_true
#define MxcBool_RetFalse() return INCREF(&_mxc_false), &_mxc_false

MxcString *true_tostring(MxcObject *);
MxcString *false_tostring(MxcObject *);

#endif
