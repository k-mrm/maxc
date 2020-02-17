#ifndef MXC_FUNCTIONOBJECT_H
#define MXC_FUNCTIONOBJECT_H

#include "function.h"
#include "builtins.h"
#include "object/object.h"
#include "frame.h"
#include "module.h"

typedef struct CallableObject CallableObject;
typedef int (*callfn_t)(CallableObject *, Frame *, size_t);

struct CallableObject {
    OBJECT_HEAD;
    callfn_t call;
};

#define CALLABLE_HEAD CallableObject head;

typedef struct FunctionObject {
    CALLABLE_HEAD;
    userfunction *func;
} FunctionObject;

typedef struct CFuncObject {
    CALLABLE_HEAD;
    CFunction func;
} CFuncObject;

FunctionObject *new_functionobject(userfunction *);
CFuncObject *new_cfnobject(CFunction);

#endif
