#include <stdio.h>
#include "object/mfiber.h"
#include "function.h"
#include "frame.h"

MxcValue new_mfiber(userfunction *uf, Frame *f) {
  MFiber *fib = (MFiber *)mxc_alloc(sizeof(MFiber));
  fib->frame = new_frame(uf, f);
  fib->frame->fiber = fib;

  return mval_obj(fib);
}

MxcValue fiber_yield(Frame *f, MxcValue *args, size_t nargs) {
  ;
}

MxcValue mfiber_yield(Frame *f, MxcValue *args, size_t nargs) {
  return fiber_yield(f, args, nargs);
}

MxcValue fiber_resume(Frame *f, MFiber *fib, MxcValue *arg, size_t nargs) {
  ;
}

MxcValue mfiber_resume(Frame *f, MxcValue *args, size_t nargs) {
  return fiber_resume(f, (MFiber *)V2O(args[0]), args, nargs);
}
