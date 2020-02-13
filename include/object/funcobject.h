#ifndef MXC_FUNCTIONOBJECT_H
#define MXC_FUNCTIONOBJECT_H

#include "function.h"
#include "builtins.h"
#include "object/object.h"
#include "frame.h"

typedef MxcObject *(*callfn_t)(MxcObject *, Frame *, MxcObject **, size_t);

typedef struct CallableObject {
    OBJECT_HEAD;
    callfn_t call;
} CallableObject;

#define CALLABLE_HEAD CallableObject head;

typedef struct FunctionObject {
    CALLABLE_HEAD;
    userfunction *func;
} FunctionObject;

typedef struct BltinFuncObject {
    CALLABLE_HEAD;
    bltinfn_ty func;
} BltinFuncObject;

FunctionObject *new_functionobject(userfunction *);
BltinFuncObject *new_bltinfnobject(bltinfn_ty);

StringObject *bltinfn_tostring(MxcObject *);
StringObject *userfn_tostring(MxcObject *);

#endif
