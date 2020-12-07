#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "object/mfiber.h"
#include "object/system.h"
#include "internal.h"
#include "function.h"
#include "context.h"
#include "mlibapi.h"
#include "vm.h"
#include "mem.h"

MxcValue new_mfiber(userfunction *uf, MContext *c) {
  MFiber *fib = (MFiber *)mxc_alloc(sizeof(MFiber));
  fib->ctx = new_econtext(uf->code, uf->nlvars, uf->d, c);
  fib->state = CREATED;
  fib->ctx->fiber = fib;
  SYSTEM(fib) = &fiber_sys;

  return mval_obj(fib);
}

MxcValue fiber_yield(MContext *c, MxcValue *args, size_t nargs) {
  if(!c->fiber) {
    panic("NULL context");
  }
  c->fiber->state = SUSPENDING;
  return args[0];
}

MxcValue mfiber_yield(MxcValue *args, size_t nargs) {
  MContext *c = curvm()->ctx;
  return fiber_yield(c, args, nargs);
}

MxcValue fiber_resume(MxcObject *f) {
  MFiber *fib = (MFiber *)f;
  VM *vm = curvm();

  switch(fib->state) {
    case RUNNING:
    case DEAD:
      return mval_invalid;
    default: break;
  }

  fib->state = RUNNING;
  MContext *ctx = vm->ctx;
  vm->ctx = fib->ctx;

  int r = (int)(intptr_t)vm_exec(vm);

  vm->ctx = ctx;
  MxcValue result = POP();

  if(!r) {
    fib->state = DEAD;
    return mval_invalid;
  }

  return result;
}

static MxcValue fiber_dead(MxcObject *f) {
  return ((MFiber *)f)->state == DEAD? mval_true : mval_false;
}

MxcValue fiber_tostring(MxcObject *ob) {
  MFiber *f = (MFiber *)ob;
  char buf[128] = {0};
  char *state;
  switch(f->state) {
    case CREATED:     state = "created"; break;
    case RUNNING:     state = "running"; break;
    case SUSPENDING:  state = "suspending"; break;
    case DEAD:        state = "dead"; break;
  }

  sprintf(buf, "%s fiber@%p", state, f);
  char *s = malloc(strlen(buf) + 1);
  strcpy(s, buf);
  return new_string(s, strlen(buf));
}

static void fiber_dealloc(MxcObject *ob) {
  MFiber *f = (MFiber *)ob;
  delete_context(f->ctx);
  Mxc_free(f);
}

static void fiber_gc_mark(MxcObject *ob) {
  OBJGCMARK(ob);
  MFiber *f = (MFiber *)ob;
  MContext *c = f->ctx;
  MxcValue v;
  while(c && c->fiber) {
    for(size_t i = 0; i < c->nlvars; i++) {
      v = c->lvars[i];
      mgc_mark(v);
    }
    c = c->prev;
  }
}

static void fiber_gc_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
  MFiber *f = (MFiber *)ob;
  MContext *c = f->ctx;
  MxcValue v;
  while(c && c->fiber) {
    for(size_t i = 0; i < c->nlvars; i++) {
      v = c->lvars[i];
      mgc_guard(v);
    }
    c = c->prev;
  }
}

static void fiber_gc_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
  MFiber *f = (MFiber *)ob;
  MContext *c = f->ctx;
  MxcValue v;
  while(c && c->fiber) {
    for(size_t i = 0; i < c->nlvars; i++) {
      v = c->lvars[i];
      mgc_unguard(v);
    }
    c = c->prev;
  }
}

static MxcValue fiber_get(MxcObject *f) {
  return mval_obj(f);
}

struct mobj_system fiber_sys = {
  "fiber",
  fiber_tostring,
  fiber_dealloc,
  0,
  fiber_gc_mark,
  fiber_gc_guard,
  fiber_gc_unguard,
  0,
  0,
  fiber_get,
  fiber_resume,
  fiber_dead,
};
