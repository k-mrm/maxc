#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mlib.h"
#include "module.h"
#include "error/error.h"
#include "object/object.h"
#include "object/intobject.h"
#include "vm.h"
#include "mem.h"
#include "frame.h"
#include "gc.h"
#include "module/module.h"

MxcValue print_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  for(int i = narg - 1; i >= 0; --i) {
    MxcValue ob = sp[i];
    MxcString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }

  return mval_null;
}

MxcValue println_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  for(int i = narg - 1; i >= 0; --i) {
    MxcValue ob = sp[i];
    MxcString *strob = ostr(mval2str(ob));
    printf("%s", strob->str);
  }
  putchar('\n');

  return mval_null;
}

MxcValue strlen_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(narg);
  MxcString *ob = ostr(sp[0]);
  int len = ITERABLE(ob)->length;
  return mval_int(len);
}

MxcValue int_tofloat_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(narg);
  MxcValue val = sp[0];
  double fnum = (double)val.num;

  return mval_float(fnum);
}

MxcValue object_id_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(narg);
  MxcValue ob = sp[0];
  intptr_t id = (intptr_t)optr(ob);
  DECREF(ob);

  return mval_int(id);
}

MxcValue sys_exit_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(narg);
  MxcValue i = sp[0];
  exit(i.num);

  return mval_null;
}

MxcValue readline_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
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

MxcValue list_len_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(narg);
  MxcList *ob = olist(sp[0]);

  return mval_int(ITERABLE(ob)->length);
}

MxcValue gc_run_core(Frame *f, MxcValue *sp, size_t narg) {
  INTERN_UNUSE(f);
  INTERN_UNUSE(sp);
  INTERN_UNUSE(narg);
  gc_run();

  return mval_null;
}

MxcModule *std_init() {
  MxcModule *module = new_mxcmodule("std");
  Global_Cbltins = new_vector();

  define_cfunc(module, "print", print_core, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(module, "println", println_core, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(module, "echo", println_core, mxcty_none, mxcty_any_vararg, NULL);
  define_cfunc(module, "len", strlen_core, mxcty_int, mxcty_string, NULL);
  define_cfunc(module, "tofloat", int_tofloat_core, mxcty_float, mxcty_int, NULL);
  define_cfunc(module, "objectid", object_id_core, mxcty_int, mxcty_any, NULL);
  define_cfunc(module, "exit", sys_exit_core, mxcty_none, mxcty_int, NULL);
  define_cfunc(module, "readline", readline_core, mxcty_string, NULL);
  define_cfunc(module, "gc_run", gc_run_core, mxcty_none, NULL);

  return module;
}

