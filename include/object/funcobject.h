#ifndef MXC_FUNCTIONOBJECT_H
#define MXC_FUNCTIONOBJECT_H

#include "function.h"
#include "builtins.h"
#include "object/object.h"
#include "frame.h"
#include "module.h"

typedef struct MxcCallable MxcCallable;
typedef int (*callfn_t)(MxcCallable *, Frame *, size_t);

struct MxcCallable {
  OBJECT_HEAD;
  callfn_t call;
};

#define CALLABLE_HEAD MxcCallable head;

typedef struct MxcFunction {
  CALLABLE_HEAD;
  userfunction *func;
} MxcFunction;

typedef struct MxcCFunc {
  CALLABLE_HEAD;
  CFunction func;
} MxcCFunc;

MxcValue new_function(userfunction *);
MxcValue new_cfunc(CFunction);

#endif
