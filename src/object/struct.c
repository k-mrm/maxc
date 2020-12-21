#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object/object.h"
#include "object/system.h"
#include "error/error.h"
#include "object/mstruct.h"
#include "mem.h"
#include "vm.h"

struct mobj_system strct_sys;

MxcValue new_struct(int nfield) {
  MStrct *ob = (MStrct *)mxc_alloc(sizeof(MStrct));
  ob->field = malloc(sizeof(MxcValue) * nfield);
  ob->nfield = nfield;
  SYSTEM(ob) = &strct_sys;
  
  return mval_obj(ob);
}

void strct_dealloc(MxcObject *ob) {
  MStrct *s = (MStrct *)ob;

  free(s->field);
  Mxc_free(s);
}

void strct_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  OBJGCMARK(ob);
  MStrct *s = (MStrct *)ob;

  for(int i = 0; i < s->nfield; i++) {
    mgc_mark(s->field[i]);
  }
}

void strct_guard(MxcObject *ob) {
  OBJGCGUARD(ob);
  MStrct *s = (MStrct *)ob;

  for(int i = 0; i < s->nfield; i++) {
    mgc_guard(s->field[i]);
  }
}

void strct_unguard(MxcObject *ob) {
  OBJGCUNGUARD(ob);
  MStrct *s = (MStrct *)ob;

  for(int i = 0; i < s->nfield; i++) {
    mgc_unguard(s->field[i]);
  }
}

MxcValue strct_copy(MxcObject *s) {
  MStrct *ob = (MStrct *)mxc_alloc(sizeof(MStrct));
  memcpy(ob, s, sizeof(MStrct));

  MxcValue *old_f = ob->field;
  ob->field = malloc(sizeof(MxcValue) * ob->nfield);
  for(size_t i = 0; i < ob->nfield; ++i) {
    ob->field[i] = mval_copy(old_f[i]);
  }

  return mval_obj(ob);
}

MxcValue strct_tostring(MxcObject *ob) {
  MStrct *s = (MStrct *)ob;
  if(s->nfield == 0) {
    return new_string_static("{}", 2);
  }
  GC_GUARD(s);
  MxcValue res = new_string_static("{", 1);
  mgc_guard(res);
  for(int i = 0; i < s->nfield; i++) {
    if(i > 0) {
      str_cstr_append(ostr(res), ", ", 2);
    }
    str_append(ostr(res), ostr(mval2str(s->field[i])));
  }
  str_cstr_append(ostr(res), "}", 1);

  GC_UNGUARD(s);
  mgc_unguard(res);

  return res;
}

struct mobj_system strct_sys = {
  "struct",
  strct_tostring,
  strct_dealloc,
  strct_copy,
  strct_gc_mark,
  strct_guard,
  strct_unguard,
  0,
  0,
  0,
  0,
  0,
  0,
};
