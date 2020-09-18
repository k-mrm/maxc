/* implementation of list object */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object/mlist.h"
#include "object/mexception.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcValue new_list(size_t size) {
  MxcList *ob = (MxcList *)mxc_alloc(sizeof(MxcList));
  SYSTEM(ob) = &list_sys;

  ob->elem = malloc(sizeof(MxcValue) * size);
  ITERABLE(ob)->length = size;

  return mval_obj(ob);
}

MxcValue list_copy(MxcObject *l) {
  MxcList *ob = (MxcList *)mxc_alloc(sizeof(MxcList));
  memcpy(ob, l, sizeof(MxcList));

  MxcValue *old = ob->elem;
  ob->elem = malloc(sizeof(MxcValue) * ITERABLE(ob)->length);
  for(size_t i = 0; i < ITERABLE(ob)->length; ++i) {
    ob->elem[i] = mval_copy(old[i]);
  }

  return mval_obj(ob);
}

MxcValue new_list_with_size(MxcValue size, MxcValue init) {
  MxcList *ob = (MxcList *)mxc_alloc(sizeof(MxcList));
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

MxcValue list_get(MxcIterable *self, int64_t idx) {
  MxcList *list = (MxcList *)self;
  if(ITERABLE(list)->length <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }

  return list->elem[idx];
}

MxcValue list_set(MxcIterable *self, int64_t idx, MxcValue a) {
  MxcList *list = (MxcList *)self;
  if(ITERABLE(list)->length <= idx) {
    mxc_raise(EXC_OUTOFRANGE, "out of range");
    return mval_invalid;
  }
  list->elem[idx] = a;

  return a;
}

void list_dealloc(MxcObject *ob) {
  MxcList *l = (MxcList *)ob;

  Mxc_free(ob);
}

void list_gc_mark(MxcObject *ob) {
  if(ob->marked) return;
  MxcList *l = (MxcList *)ob;

  ob->marked = 1;
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_mark(l->elem[i]);
  }
}

void list_guard(MxcObject *ob) {
  MxcList *l = (MxcList *)ob;

  ob->gc_guard = 1;
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_guard(l->elem[i]);
  }
}

void list_unguard(MxcObject *ob) {
  MxcList *l = (MxcList *)ob;

  ob->gc_guard = 0;
  for(size_t i = 0; i < ITERABLE(l)->length; ++i) {
    mgc_unguard(l->elem[i]);
  }
}

MxcValue list_tostring(MxcObject *ob) {
  MxcValue res;
  MxcList *l = (MxcList *)ob;
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
