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
  double fnum = (double)V2I(val);

  return mval_float(fnum);
}

static MxcValue object_id(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue ob = sp[0];
#ifdef NAN_BOXING
  intptr_t id = (intptr_t)ob.raw;
#else
  intptr_t id = (intptr_t)V2O(ob);
#endif

  return mval_int(id);
}

static MxcValue sys_exit(MxcValue *sp, size_t narg) __attribute__((noreturn));

static MxcValue sys_exit(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(narg);
  MxcValue i = sp[0];

  exit(V2I(i));
}

static MxcValue mgc_run(MxcValue *sp, size_t narg) {
  INTERN_UNUSE(sp);
  INTERN_UNUSE(narg);

  gc_run();

  return mval_null;
}

MxcValue margv;

void setup_argv(int argc, char **argv) {
  margv = new_list(argc);
  for(int i = 0; i < argc; i++) {
    char *a_cstr = argv[i];
    MxcValue a = new_string(a_cstr, strlen(a_cstr));
    listadd((MList *)V2O(margv), a);
  }
}

void std_init() {
  MxcModule *mod = new_mxcmodule("std");

  define_cfunc(mod, "print", print, mxc_none, mxc_any_vararg, NULL);
  define_cfunc(mod, "println", println, mxc_none, mxc_any_vararg, NULL);
  define_cfunc(mod, "echo", println, mxc_none, mxc_any_vararg, NULL);
  define_cfunc(mod, "panic", mpanic, mxc_none, mxc_any_vararg, NULL);
  define_cfunc(mod, "tofloat", int_tofloat, mxc_float, mxc_int, NULL);
  define_cfunc(mod, "objectid", object_id, mxc_int, mxc_any, NULL);
  define_cfunc(mod, "exit", sys_exit, mxc_none, mxc_int, NULL);
  define_cfunc(mod, "gc_run", mgc_run, mxc_none, NULL);
  define_cconst(mod, "argv", margv, new_type_list(mxc_string));

  reg_gmodule(mod);
}
