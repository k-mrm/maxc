#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maxc.h"
#include "mlib.h"
#include "module.h"
#include "error/error.h"
#include "object/object.h"
#include "object/intobject.h"
#include "vm.h"
#include "mem.h"
#include "frame.h"
#include "gc.h"

static MxcValue print(MxcValue *sp, size_t narg) {
  for(int i = narg - 1; i >= 0; --i) {
    MxcValue ob = sp[i];
    MxcString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }

  return mval_null;
}

static MxcValue println(MxcValue *sp, size_t narg) {
  for(int i = narg - 1; i >= 0; --i) {
    MxcValue ob = sp[i];
    MxcString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }
  putchar('\n');

  return mval_null;
}

static MxcValue mstrlen(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcString *ob = ostr(sp[0]);
  int len = ITERABLE(ob)->length;
  return mval_int(len);
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

static MxcValue readline(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(sp);
  INTERN_UNUSE(narg);
  size_t cur;
  ReadStatus rs = intern_readline(1024, &cur, "", 0); 
  if(rs.err.eof) {
    // TODO
  }
  if(rs.err.toolong) {
    // TODO
  }

  if(rs.str) {
    return new_string(rs.str, strlen(rs.str));
  }
  else {
    return new_string_static("", 0);
  }
}

static MxcValue mgc_run(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(sp);
  INTERN_UNUSE(narg);

  gc_run();

  return mval_null;
}

void std_init() {
  MxcModule *mod = new_mxcmodule("std");

  define_cfunc(mod, "print", print, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "println", println, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "echo", println, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(mod, "len", mstrlen, mxcty_int, mxcty_string, NULL);
  define_cfunc(mod, "tofloat", int_tofloat, mxcty_float, mxcty_int, NULL);
  define_cfunc(mod, "objectid", object_id, mxcty_int, mxcty_any, NULL);
  define_cfunc(mod, "exit", sys_exit, mxcty_none, mxcty_int, NULL);
  define_cfunc(mod, "readline", readline, mxcty_string, NULL);
  define_cfunc(mod, "gc_run", mgc_run, mxcty_none, NULL);

  register_module(mod);
}