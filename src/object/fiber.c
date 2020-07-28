#include <stdio.h>
#include "object/mfiber.h"
#include "function.h"
#include "frame.h"
#include "mlibapi.h"
#include "vm.h"
#include "mem.h"

MxcValue new_mfiber(userfunction *uf, MContext *c) {
  MFiber *fib = (MFiber *)mxc_alloc(sizeof(MFiber));
  fib->ctx = new_econtext(uf, c);
  fib->ctx->fiber = fib;

  return mval_obj(fib);
}

void fiber_ctx_switch(MContext *c) {

}

MxcValue fiber_yield(MContext *c, MxcValue *args, size_t nargs) {
  c->fiber->state = SUSPENDING;
  return args[0];
}

MxcValue mfiber_yield(MContext *c, MxcValue *args, size_t nargs) {
  return fiber_yield(c, args, nargs);
}

MxcValue fiber_resume(MContext *c, MFiber *fib, MxcValue *arg, size_t nargs) {
  switch(fib->state) {
    case RUNNING:
    case DEAD:
      return mval_null;
  }
  int r = vm_exec(fib->ctx);

  return mval_null;
}

MxcValue mfiber_resume(MContext *c, MxcValue *args, size_t nargs) {
  return fiber_resume(c, (MFiber *)V2O(args[0]), args, nargs);
}
