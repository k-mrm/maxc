/* implementation of list object */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object/mlist.h"
#include "object/mexception.h"
#include "type.h"
#include "error/error.h"
#include "mlib.h"
#include "mem.h"
#include "vm.h"

MxcValue new_list(size_t size) {
  MList *ob = (MList *)mxc_alloc(sizeof(MList));
  SYSTEM(ob) = &list_sys;

  ob->elem = malloc(sizeof(MxcValue) * size);
  ITERABLE(ob)->length = size;

  return mval_obj(ob);
}

MxcValue list_copy(MxcObject *l) {
  MList *ob = (MList *)mxc_alloc(sizeof(MList));
  memcpy(ob, l, sizeof(MList));

  MxcValue *old = ob->elem;
  ob->elem = malloc(sizeof(MxcValue) * ITERABLE(ob)->length);
  for(size_t i = 0; i < ITERABLE(ob)->length; ++i) {
    ob->elem[i] = mval_copy(old[i]);
  }

  return mval_obj(ob);
}

MxcValue new_list_size(MxcValue size, MxcValue init) {
  MList *ob = (MList *)mxc_alloc(sizeof(MList));
  int64_t len = size.num;
  ITERABLE(ob)->length = len;
  SYSTEM(ob) = &list_sys;

  if(len < 0) {
    mxc_raise(EXC_OUTOFRANGE, "negative length");
    return mval_invalid;
  }

  ob->elem = malloc(sizeof(MxcValue) * len);
  MxcValue *ptr = ob->elem;
  while(len--) {
    *ptr++ = init;
  }

  return mval_obj(ob);
}

MxcValue list_get(MxcIterable *self, MxcValue index) {
  MList *list = (MList *)self;
  int64_t idx = index.num;
  if(ITERABLE(list)->length <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }

  return list->elem[idx];
}

MxcValue list_set(MxcIterable *self, MxcValue index, MxcValue a) {
  MList *list = (MList *)self;
  int64_t idx = index.num;
  if(ITERABLE(list)->length <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }
  list->elem[idx] = a;

  return a;
}

MxcValue listlen(MList *l) {
  return mval_int(ITERABLE(l)->length);
}

MxcValue mlistlen(MxcValue *a, size_t na) {
  return listlen((MList *)V2O(a[0]));
}

MxcValue listadd(MList *l, MxcValue a) {
  puts("aaaaaaaaaaaad");
  return mval_obj(l);
}

MxcValue mlistadd(MxcValue *a, size_t na) {
  return listadd((MList *)V2O(a[0]), a[1]);
}

void list_dealloc(MxcObject *ob) {
  MList *l = (MList *)ob;

  Mxc_free(ob);
}

void list_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  MList *l = (MList *)ob;

  OBJGCMARK(ob);
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_mark(l->elem[i]);
  }
}

void list_guard(MxcObject *ob) {
  MList *l = (MList *)ob;

  OBJGCGUARD(ob);
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_guard(l->elem[i]);
  }
}

void list_unguard(MxcObject *ob) {
  MList *l = (MList *)ob;

  OBJGCUNGUARD(ob);
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_unguard(l->elem[i]);
  }
}

MxcValue list_tostring(MxcObject *ob) {
  MxcValue res;
  MList *l = (MList *)ob;
  GC_GUARD(l);

  if(ITERABLE(l)->length == 0) {
    res = new_string_static("[]", 2);
    GC_UNGUARD(l);
    return res;
  }

  res = new_string_static("[", 1);
  mgc_guard(res);
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    if(i > 0)
      str_cstr_append(ostr(res), ", ", 2);

    MxcValue elemstr = mval2str(l->elem[i]);
    str_append(ostr(res), ostr(elemstr));
  }

  str_cstr_append(ostr(res), "]", 1);

  GC_UNGUARD(l);
  mgc_unguard(res);

  return res;
}

struct mobj_system list_sys = {
  "list",
  list_tostring,
  list_dealloc,
  list_copy,
  list_gc_mark,
  list_guard,
  list_unguard,
  list_get,
  list_set,
  iterable_reset,
  iterable_next,
  iterable_stopped,
};

void listlib_init(MInterp *m) {
  MxcModule *mod = new_mxcmodule("list");

  Type *tlist = new_type_list(mxc_any);
  define_cfunc(mod, "len", mlistlen, FTYPE(mxc_int, tlist));
  Type *vart = typevar("T");
  Type *lvart = new_type_list(vart);
  define_cfunc(mod, "add", mlistadd, FTYPE(lvart, lvart, vart));

  register_module(m, mod);
}
