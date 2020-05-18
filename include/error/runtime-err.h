#ifndef MXC_RUNTIME_ERR_H
#define MXC_RUNTIME_ERR_H

#include "error/errortype.h"
#include "object/object.h"

struct Frame;
struct MxcObject;
typedef struct MxcObject MxcObject;
typedef struct Frame Frame;

typedef struct RuntimeErr {
  enum RuntimeErrType type;    
  MxcValue args[2];
  int argc;
} RuntimeErr;

void mxc_raise_err(Frame *frame, enum RuntimeErrType);
void raise_outofrange(Frame *, MxcValue, MxcValue);
void runtime_error(Frame *);

#endif
