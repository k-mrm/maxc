/* implementation of function object */
#include <string.h>
#include <stdlib.h>
#include "object/object.h"
#include "object/system.h"
#include "object/mfunc.h"
#include "object/mfiber.h"
#include "error/error.h"
#include "mem.h"
#include "gc.h"
#include "vm.h"
#include "execarg.h"

int userfn_call(MCallable *self, MContext *c, size_t nargs) {
  INTERN_UNUSE(nargs);

  VM *vm = curvm();
  MxcFunction *callee = (MxcFunction *)self;
  userfunction *f = callee->func;
  vm->ctx = new_econtext(f->code, f->nlvars, f->d, c);
  int res = (int)(intptr_t)vm_exec(vm);

  delete_context(vm->ctx);
  vm->ctx = c;
  return res;
}

MxcValue new_function(userfunction *u, bool iter) {
  NEW_OBJECT(MxcFunction, ob, userfn_sys);

  ob->func = u;
  ob->iter = iter;
  ((MCallable *)ob)->call = userfn_call;

  return mval_obj(ob);
}

MxcValue userfn_copy(MxcObject *u) {
  MxcFunction *n = (MxcFunction *)mxc_alloc(sizeof(MxcFunction));
  memcpy(n, u, sizeof(MxcFunction));

  return mval_obj(n);
}

static void userfn_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
}

static void userfn_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
}

static void userfn_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
}

static void userfn_dealloc(MxcObject *ob) {
  free(((MxcFunction *)ob)->func);
  Mxc_free(ob);
}

int cfn_call(MCallable *self, MContext *context, size_t nargs) {
  extern enum stack_cache_state scstate;

  VM *vm = curvm();
  MxcCFunc *callee = (MxcCFunc *)self;
  MxcValue *stack_argv = vm->stackptr - nargs;

  /*
  int ncache = SC_NCACHE();
  int dec_sp = nargs - ncache;
  if(dec_sp < 0) {
    dec_sp = 0;
  }
  stack_argv -= dec_sp;

  if(ncache && dec_sp)
    memmove(stack_argv + ncache, stack_argv, dec_sp);

  MxcValue top;
  MxcValue second;
  if(ncache == 2) {
    if(SC_TOPA()) {
      top = screg_a;
      second = screg_b;
    }
    else {
      top = screg_b;
      second = screg_a;
    }

    if(nargs >= 2) {
      stack_argv[0] = top;
      stack_argv[1] = second;
    }
    else if(nargs == 1) {
      stack_argv[0] = top;
    }
  }
  else if(ncache == 1) {
    if(SC_TOPA()) {
      top = screg_a;
    }
    else {
      top = screg_b;
    }

    if(nargs >= 1) {
      stack_argv[0] = top;
    }
  }
  */

  MxcValue ret = callee->func(stack_argv, nargs);
  vm->stackptr = stack_argv;
  screg_a = ret;
  scstate = SCAX;

  return 0;
}

MxcValue new_cfunc(cfunction cf) {
  NEW_OBJECT(MxcCFunc, ob, cfn_sys);

  ob->func = cf;
  ((MCallable *)ob)->call = cfn_call;

  return mval_obj(ob);
}

MxcValue cfn_copy(MxcObject *b) {
  MxcCFunc *n = (MxcCFunc *)mxc_alloc(sizeof(MxcCFunc));
  memcpy(n, b, sizeof(MxcCFunc));
  INCREF(b);

  return mval_obj(n);
}

static void cfn_dealloc(MxcObject *ob) {
  Mxc_free(ob);
}

static void cfn_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
}

static void cfn_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
}

static void cfn_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
}

static MxcValue userfn_tostring(MxcObject *ob) {
  char *s = malloc(sizeof(char *) * 64);
  int len = sprintf(s, "<user-def function at %p>", ob);

  return new_string(s, (size_t)len);
}

static MxcValue cfn_tostring(MxcObject *ob) {
  char *s = malloc(sizeof(char *) * 64);
  int len = sprintf(s, "<builtin function at %p>", ob);

  return new_string(s, (size_t)len);
}

struct mobj_system userfn_sys = {
  "user-def function",
  NULL,
  userfn_tostring,
  userfn_dealloc,
  userfn_copy,
  userfn_mark,
  userfn_guard,
  userfn_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
};

struct mobj_system cfn_sys = {
  "builtin function",
  NULL,
  cfn_tostring,
  cfn_dealloc,
  cfn_copy,
  cfn_mark,
  cfn_guard,
  cfn_unguard,
  0,
  0,
  0,
  0,
  0,
  obj_hash32,
};

