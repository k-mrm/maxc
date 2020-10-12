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

static MxcValue print(MxcValue *sp, size_t narg) {
  for(int i = 0; i < narg; i++) {
    MxcValue ob = sp[i];
    MString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }

  return mval_null;
}

static MxcValue println(MxcValue *sp, size_t narg) {
  MString *strob;
  if(narg == 0) {
    printf("\n");
    return mval_null;
  }

  for(int i = 0; i < narg; i++) {
    MxcValue ob = sp[i];
    strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }

  if(strob->str[ITERABLE(strob)->length - 1] != '\n')
    printf("\n");

  return mval_null;
}

static MxcValue mpanic(MxcValue *sp, size_t narg) {
  fprintf(stderr, "program panicked ");

  MString *strob;
  if(narg == 0) {
    fprintf(stderr, "\n");
    exit(1);
  }

  for(int i = 0; i < narg; i++) {
    MxcValue ob = sp[i];
    strob = ostr(mval2str(ob));
    fprintf(stderr, "%s", strob->str);
  }

  if(strob->str[ITERABLE(strob)->length - 1] != '\n')
    fprintf(stderr, "\n");

  exit(1);
}

static MxcValue int_tofloat(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue val = sp[0];
  double fnum = (double)val.num;

  return mval_float(fnum);
}

static MxcValue object_id(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue ob = sp[0];
  intptr_t id = (intptr_t)V2O(ob);
  DECREF(ob);

  return mval_int(id);
}

static MxcValue sys_exit(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue i = sp[0];

  exit(i.num);

  return mval_null;
}

static MxcValue mgc_run(MxcValue *sp, size_t narg) {
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
  define_cfunc(mod, "panic", mpanic, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "tofloat", int_tofloat, mxcty_float, mxcty_int, NULL);
  define_cfunc(mod, "objectid", object_id, mxcty_int, mxcty_any, NULL);
  define_cfunc(mod, "exit", sys_exit, mxcty_none, mxcty_int, NULL);
  define_cfunc(mod, "gc_run", mgc_run, mxcty_none, NULL);

  register_module(m, mod);
}
