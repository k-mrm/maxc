/* implementation of function object */
#include <string.h>
#include <stdlib.h>

#include "object/object.h"
#include "object/funcobject.h"
#include "error/error.h"
#include "mem.h"
#include "gc.h"
#include "vm.h"

int userfn_call(MCallable *self,
    MContext *f,
    size_t nargs) {
  INTERN_UNUSE(nargs);
  MxcFunction *callee = (MxcFunction *)self;
  MContext *new = new_econtext(callee->func, f);
  int res = vm_exec(new);

  /*
     for(size_t i = 0; i < new->nlvars; ++i) {
     if(new->lvars[i])
     DECREF(new->lvars[i]);
     }
     */

  f->stackptr = new->stackptr;
  delete_frame(new);

  cur_frame = f;

  return res;
}

MxcValue new_function(userfunction *u) {
  MxcFunction *ob = (MxcFunction *)mxc_alloc(sizeof(MxcFunction));
  ob->func = u;
  ((MCallable *)ob)->call = userfn_call;
  SYSTEM(ob) = &userfn_sys;

  return mval_obj(ob);
}

MxcValue userfn_copy(MxcObject *u) {
  MxcFunction *n = (MxcFunction *)mxc_alloc(sizeof(MxcFunction));
  memcpy(n, u, sizeof(MxcFunction));
  INCREF(u);

  return mval_obj(n);
}

void userfn_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
}

void userfn_guard(MxcObject *ob) {
  ob->gc_guard = 1;
}

void userfn_unguard(MxcObject *ob) {
  ob->gc_guard = 0;
}

void userfn_dealloc(MxcObject *ob) {
  free(((MxcFunction *)ob)->func);
  Mxc_free(ob);
}

int cfn_call(MCallable *self,
    MContext *frame,
    size_t nargs) {
  MxcCFunc *callee = (MxcCFunc *)self;
  MxcValue *args = frame->stackptr - nargs;
  MxcValue ret = callee->func(frame, args, nargs);
  frame->stackptr = args;
  Push(ret);

  return 0;
}

MxcValue new_cfunc(cfunction cf) {
  MxcCFunc *ob =
    (MxcCFunc *)mxc_alloc(sizeof(MxcCFunc));
  ob->func = cf;
  ((MCallable *)ob)->call = cfn_call;
  SYSTEM(ob) = &cfn_sys;

  return mval_obj(ob);
}

MxcValue cfn_copy(MxcObject *b) {
  MxcCFunc *n = (MxcCFunc *)mxc_alloc(sizeof(MxcCFunc));
  memcpy(n, b, sizeof(MxcCFunc));
  INCREF(b);

  return mval_obj(n);
}

void cfn_dealloc(MxcObject *ob) {
  Mxc_free(ob);
}

void cfn_mark(MxcObject *ob) {
  if(ob->marked) return;
  ob->marked = 1;
}

void cfn_guard(MxcObject *ob) {
  ob->gc_guard = 1;
}

void cfn_unguard(MxcObject *ob) {
  ob->gc_guard = 0;
}

MxcValue userfn_tostring(MxcObject *ob) {
  char *s = malloc(sizeof(char *) * 64);
  int len = sprintf(s, "<user-def function at %p>", ob);

  return new_string(s, (size_t)len);
}

MxcValue cfn_tostring(MxcObject *ob) {
  char *s = malloc(sizeof(char *) * 64);
  int len = sprintf(s, "<builtin function at %p>", ob);

  return new_string(s, (size_t)len);
}

struct mobj_system userfn_sys = {
  "user-def function",
  userfn_tostring,
  userfn_dealloc,
  userfn_copy,
  userfn_mark,
  userfn_guard,
  userfn_unguard,
  0,
  0,
};

struct mobj_system cfn_sys = {
  "builtin function",
  cfn_tostring,
  cfn_dealloc,
  cfn_copy,
  cfn_mark,
  cfn_guard,
  cfn_unguard,
  0,
  0,
};

