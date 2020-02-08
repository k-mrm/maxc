#ifndef MXC_NULLOBJECT_H
#define MXC_NULLOBJECT_H

#include "object/object.h"

typedef struct NullObject {
    OBJECT_HEAD;
} NullObject;

extern NullObject MxcNull;
#define MXC_NULL  ((MxcObject *)&MxcNull)

#define Mxc_RetNull() return INCREF(&MxcNull), MXC_NULL

StringObject *null_tostring(MxcObject *);

#endif
