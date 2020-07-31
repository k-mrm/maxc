#ifndef MXC_MFIBER_H
#define MXC_MFIBER_H

#include "object/object.h"
#include "frame.h"

enum fiberstate {
  CREATED,
  SUSPENDING,
  RUNNING,
  DEAD,
};

typedef struct MFiber MFiber;
struct MFiber {
  OBJECT_HEAD;
  enum fiberstate state;
  MContext *ctx;
};

MxcValue new_mfiber(userfunction *, MContext *);
MxcValue fiber_resume(MContext *, MFiber *, MxcValue *, size_t);
MxcValue fiber_yield(MContext *, MxcValue *, size_t);

#endif
