#include <stdio.h>
#include "object/mfiber.h"
#include "function.h"
#include "frame.h"

MxcValue new_mfiber(userfunction *uf, Frame *f) {
  MFiber *fib = (MFiber *)Mxc_malloc(sizeof(MFiber));
  fib->frame = new_frame(uf, f);
  fib->frame->fiber = fib;

  return mval_obj(fib);
}
