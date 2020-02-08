#ifndef MXC_FUNCTIONOBJECT_H
#define MXC_FUNCTIONOBJECT_H

#include "function.h"
#include "builtins.h"
#include "object/object.h"

typedef struct FunctionObject {
    OBJECT_HEAD;
    userfunction *func;
} FunctionObject;

typedef struct BltinFuncObject {
    OBJECT_HEAD;
    bltinfn_ty func;
} BltinFuncObject;

FunctionObject *new_functionobject(userfunction *);
BltinFuncObject *new_bltinfnobject(bltinfn_ty);

StringObject *bltinfn_tostring(MxcObject *);
StringObject *userfn_tostring(MxcObject *);

#endif
