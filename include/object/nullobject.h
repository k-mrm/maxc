#ifndef MXC_NULLOBJECT_H
#define MXC_NULLOBJECT_H

#include "object/object.h"

typedef struct MxcNull {
    OBJECT_HEAD;
} MxcNull;

extern MxcNull _mxc_null;

#define MXC_NULL  ((MxcObject *)&_mxc_null)
#define Mxc_RetNull() return INCREF(&mxc_null), MXC_NULL

MxcString *null_tostring(MxcObject *);

#endif
