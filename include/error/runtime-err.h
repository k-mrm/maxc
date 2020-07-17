#ifndef MXC_RUNTIME_ERR_H
#define MXC_RUNTIME_ERR_H

#include "error/errortype.h"
#include "object/object.h"

struct MContext;
struct MxcObject;
typedef struct MxcObject MxcObject;
typedef struct MContext MContext;

typedef struct RuntimeErr {
  enum RuntimeErrType type;    
  MxcValue args[2];
  int argc;
} RuntimeErr;

void mxc_raise_err(MContext *frame, enum RuntimeErrType);
void raise_outofrange(MContext *, MxcValue, MxcValue);
void runtime_error(MContext *);

#endif
