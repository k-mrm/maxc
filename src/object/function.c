/* implementation of function object */
#include <string.h>
#include <stdlib.h>

#include "object/object.h"
#include "object/funcobject.h"
#include "error/error.h"
#include "mem.h"
#include "gc.h"
#include "vm.h"

int userfn_call(MxcCallable *self,
    Frame *f,
    size_t nargs) {
  INTERN_UNUSE(nargs);
  MxcFunction *callee = (MxcFunction *)self;
  Frame *new = new_frame(callee->func, f);
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
  MxcFunction *ob = (MxcFunction *)Mxc_malloc(sizeof(MxcFunction));
  ob->func = u;
  ((MxcCallable *)ob)->call = userfn_call;
  OBJIMPL(ob) = &userfn_objimpl;

  return mval_obj(ob);
}

MxcValue userfn_copy(MxcObject *u) {
  MxcFunction *n = (MxcFunction *)Mxc_malloc(sizeof(MxcFunction));
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

int cfn_call(MxcCallable *self,
    Frame *frame,
    size_t nargs) {
  MxcCFunc *callee = (MxcCFunc *)self;
  MxcValue *args = frame->stackptr - nargs;
  MxcValue ret = callee->func(frame, args, nargs);
  frame->stackptr = args;
  Push(ret);

  return 0;
}

MxcValue new_cfunc(CFunction cf) {
  MxcCFunc *ob =
    (MxcCFunc *)Mxc_malloc(sizeof(MxcCFunc));
  ob->func = cf;
  ((MxcCallable *)ob)->call = cfn_call;
  OBJIMPL(ob) = &cfn_objimpl;

  return mval_obj(ob);
}

MxcValue cfn_copy(MxcObject *b) {
  MxcCFunc *n =
    (MxcCFunc *)Mxc_malloc(sizeof(MxcCFunc));
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
  (void)ob;
  char *str = "<builtin function>";
  return new_string_static(str, strlen(str));
}

MxcObjImpl userfn_objimpl = {
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

MxcObjImpl cfn_objimpl = {
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

