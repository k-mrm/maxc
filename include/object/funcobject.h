#ifndef MXC_FUNCTIONOBJECT_H
#define MXC_FUNCTIONOBJECT_H

#include "function.h"
#include "object/object.h"
#include "frame.h"

typedef struct MCallable MCallable;

typedef int (*callfn_t)(MCallable *, MContext *, size_t);
typedef struct MxcValue (*cfunction)(MContext *, MxcValue *, size_t);

struct MCallable {
  OBJECT_HEAD;
  callfn_t call;
};

#define CALLABLE_HEAD MCallable head;

typedef struct MxcFunction {
  CALLABLE_HEAD;
  userfunction *func;
} MxcFunction;

typedef struct MxcCFunc {
  CALLABLE_HEAD;
  cfunction func;
} MxcCFunc;

MxcValue new_function(userfunction *);
MxcValue new_cfunc(cfunction);

#endif
