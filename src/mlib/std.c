#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maxc.h"
#include "mlib.h"
#include "mlibapi.h"
#include "error/error.h"
#include "object/object.h"
#include "object/mint.h"
#include "vm.h"
#include "mem.h"
#include "context.h"
#include "gc.h"

static MxcValue print(MContext *f, MxcValue *sp, size_t narg) {
  for(int i = 0; i < narg; i++) {
    MxcValue ob = sp[i];
    MxcString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }

  return mval_null;
}

static MxcValue println(MContext *f, MxcValue *sp, size_t narg) {
  MxcString *strob;
  if(narg == 0) {
    putchar('\n');
    return mval_null;
  }

  for(int i = 0; i < narg; i++) {
    MxcValue ob = sp[i];
    strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }
  printf(strob->str[ITERABLE(strob)->length - 1] == '\n'? "" : "\n");

  return mval_null;
}

static MxcValue mstrlen(MContext *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcString *ob = ostr(sp[0]);
  int len = ITERABLE(ob)->length;
  return mval_int(len);
}

static MxcValue int_tofloat(MContext *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue val = sp[0];
  double fnum = (double)val.num;

  return mval_float(fnum);
}

static MxcValue object_id(MContext *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue ob = sp[0];
  intptr_t id = (intptr_t)V2O(ob);
  DECREF(ob);

  return mval_int(id);
}

static MxcValue sys_exit(MContext *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue i = sp[0];
  exit(i.num);

  return mval_null;
}

static MxcValue mgc_run(MContext *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(sp);
  INTERN_UNUSE(narg);

  gc_run();

  return mval_null;
}

void std_init(MInterp *m) {
  MxcModule *mod = new_mxcmodule("std");

  define_cfunc(mod, "print", print, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "println", println, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "echo", println, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "len", mstrlen, mxcty_int, mxcty_string, NULL);
  define_cfunc(mod, "tofloat", int_tofloat, mxcty_float, mxcty_int, NULL);
  define_cfunc(mod, "objectid", object_id, mxcty_int, mxcty_any, NULL);
  define_cfunc(mod, "exit", sys_exit, mxcty_none, mxcty_int, NULL);
  define_cfunc(mod, "gc_run", mgc_run, mxcty_none, NULL);

  register_module(m, mod);
}
