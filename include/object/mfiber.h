#ifndef MXC_MFIBER_H
#define MXC_MFIBER_H

#include "object/object.h"
#include "frame.h"

enum fiberstate {
  SUSPEND,
  RUNNING,
  DEAD,
};

typedef struct MFiber MFiber;
struct MFiber {
  OBJECT_HEAD;
  enum fiberstate state;
  MContext *frame;
};

#endif
