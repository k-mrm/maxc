#ifndef MXC_MFIBER_H
#define MXC_MFIBER_H

#include "object/object.h"
#include "frame.h"

typedef struct MFiber MFiber;
struct MFiber {
  OBJECT_HEAD;
  Frame *frame;
};

#endif
