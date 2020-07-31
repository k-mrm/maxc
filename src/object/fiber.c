#include <stdio.h>
#include "object/mfiber.h"
#include "function.h"
#include "frame.h"
#include "mlibapi.h"
#include "vm.h"
#include "mem.h"

MxcValue new_mfiber(userfunction *uf, MContext *c) {
  MFiber *fib = (MFiber *)mxc_alloc(sizeof(MFiber));
  fib->ctx = new_econtext(uf->code, uf->nlvars, uf->name, c);
  fib->state = CREATED;
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
  VM *vm = curvm();

  switch(fib->state) {
    case RUNNING:
    case DEAD:
      return mval_invalid;
    default: break;
  }

  stack_dump("fiber resume now");
  fib->state = RUNNING;
  MContext *ctx = vm->ctx;
  vm->ctx = fib->ctx;
  int r = vm_exec();
  if(!r) fib->state = DEAD;
  vm->ctx = ctx;
  stack_dump("fiber resume waaaas");

  return TOP();
}

MxcValue mfiber_resume(MContext *c, MxcValue *args, size_t nargs) {
  return fiber_resume(c, (MFiber *)V2O(args[0]), args, nargs);
}

