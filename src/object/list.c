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
  LISTCAPA(ob) = size;
  LISTLEN(ob) = 0;

  return mval_obj(ob);
}

MxcValue list_copy(MxcObject *l) {
  MList *ob = (MList *)mxc_alloc(sizeof(MList));
  memcpy(ob, l, sizeof(MList));

  MxcValue *old = ob->elem;
  ob->elem = malloc(sizeof(MxcValue) * LISTLEN(ob));
  for(size_t i = 0; i < LISTLEN(ob); ++i) {
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
  if(idx < 0) {
    idx = LISTLEN(list) + idx;
  }

  if(LISTLEN(list) <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }

  return list->elem[idx];
}

MxcValue list_set(MxcIterable *self, MxcValue index, MxcValue a) {
  MList *list = (MList *)self;
  int64_t idx = index.num;
  if(idx < 0) {
    idx = LISTLEN(list) + idx;
  }

  if(LISTLEN(list) <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }
  list->elem[idx] = a;

  return a;
}

MxcValue listlen(MList *l) {
  return mval_int(LISTLEN(l));
}

MxcValue mlistlen(MxcValue *a, size_t na) {
  return listlen((MList *)V2O(a[0]));
}

MxcValue listclear(MList *l) {
  LISTLEN(l) = 0;
  return mval_null;
}

static MxcValue mlistclear(MxcValue *a, size_t na) {
  return listclear((MList *)V2O(a[0]));
}

void listexpand(MList *l) {
  LISTCAPA(l) *= 2;
  l->elem = realloc(l->elem, sizeof(MxcValue) * LISTCAPA(l));
}

MxcValue listadd(MList *l, MxcValue a) {
  if(LISTLEN(l) == LISTCAPA(l)) {
    listexpand(l);
  }

  l->elem[LISTLEN(l)] = a;
  LISTLEN(l) += 1;
  return mval_obj(l);
}

MxcValue mlistadd(MxcValue *a, size_t na) {
  return listadd((MList *)V2O(a[0]), a[1]);
}

MxcValue listdel_at(MList *l, int64_t idx) {
  MxcValue *e = l->elem;
  for(int i = idx; i < LISTLEN(l) - 1; i++) {
    e[i] = e[i + 1];
  } 

  LISTLEN(l) -= 1;

  return mval_null;
}

static MxcValue mlistdel_at(MxcValue *a, size_t na) {
  return listdel_at((MList *)V2O(a[0]), V2I(a[1]));
}

MxcValue listpop(MList *l) {
  MxcValue val = l->elem[LISTLEN(l) - 1];
  LISTLEN(l) -= 1;
  return val;
}

static MxcValue mlistpop(MxcValue *a, size_t na) {
  return listpop((MList *)V2O(a[0]));
}

MxcValue listrev(MList *l) {
  MxcValue *elem = l->elem;
  int end = LISTLEN(l) - 1;

  for(int i = 0; i < end; i++, end--) {
    MxcValue tmp = elem[i];
    elem[i] = elem[end];
    elem[end] = tmp;
  }

  return mval_obj(l);
}

MxcValue mlistrev(MxcValue *a, size_t na) {
  return listrev((MList *)V2O(a[0]));
}

static MxcValue listreversed(MList *l) {
  MxcValue newl = new_list(LISTLEN(l));
  MList *n = (MList *)V2O(newl);

  for(int i = LISTLEN(l) - 1; i >= 0; i--) {
    listadd(n, l->elem[i]);
  }

  return mval_obj(n);
}

MxcValue mlistreversed(MxcValue *a, size_t na) {
  return listreversed((MList *)V2O(a[0]));
}

void list_dealloc(MxcObject *ob) {
  MList *l = (MList *)ob;

  Mxc_free(ob);
}

void list_gc_mark(MxcObject *ob) {
  if(OBJGCMARKED(ob)) return;
  MList *l = (MList *)ob;

  OBJGCMARK(ob);
  for(size_t i = 0; i < LISTLEN(l); ++i) {
    mgc_mark(l->elem[i]);
  }
}

void list_guard(MxcObject *ob) {
  MList *l = (MList *)ob;

  OBJGCGUARD(ob);
  for(size_t i = 0; i < LISTLEN(l); ++i) {
    mgc_guard(l->elem[i]);
  }
}

void list_unguard(MxcObject *ob) {
  MList *l = (MList *)ob;

  OBJGCUNGUARD(ob);
  for(size_t i = 0; i < LISTLEN(l); ++i) {
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
  Type *vart2 = typevar("T");
  Type *lvart2 = new_type_list(vart2);
  define_cfunc(mod, "reversed", mlistreversed, FTYPE(lvart2, lvart2));
  Type *vart3 = typevar("T");
  Type *lvart3 = new_type_list(vart3);
  define_cfunc(mod, "pop", mlistpop, FTYPE(vart3, lvart3));
  Type *lvart4 = new_type_list(typevar("T"));
  define_cfunc(mod, "reverse", mlistrev, FTYPE(lvart4, lvart4));
  Type *lvart5 = new_type_list(typevar("T"));
  define_cfunc(mod, "clear", mlistclear, FTYPE(mxc_none, lvart5));
  Type *lvart6 = new_type_list(typevar("T"));
  define_cfunc(mod, "del_at", mlistdel_at, FTYPE(mxc_none, lvart6, mxc_int));

  register_module(m, mod);
}
