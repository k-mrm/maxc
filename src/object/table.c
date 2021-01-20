#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "object/mtable.h"
#include "object/system.h"
#include "object/mexception.h"
#include "object/mstr.h"
#include "mem.h"

static MxcValue table_tostring(MxcObject *);

NEW_EXCEPTION(exc_unknownkey, "key error");
#define EXC_UNKNOWN_KEY   (&exc_unknownkey)

static int primes[] = {
  3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093
};

static int nslot_from(int c) {
  static int nprimes = sizeof(primes) / sizeof(int);
  for(int i = 0; i < nprimes; i++) {
    if(c < primes[i])
      return primes[i];
  }

  return 0;
}

static struct mentry *new_entry(MxcValue k, MxcValue v) {
  struct mentry *e = malloc(sizeof(struct mentry));
  e->key = k;
  e->val = v;
  e->next = NULL;
  return e;
}

MxcValue new_table_capa(int capa) {
  MTable *table = (MTable *)mxc_alloc(sizeof(MTable));
  SYSTEM(table) = &table_sys;
  int nslot = nslot_from(capa);
  table->e = calloc(1, sizeof(struct mentry *) * nslot);
  table->nslot = nslot;
  table->nentry = 0;
  table->default_val = mval_invalid;

  return mval_obj(table);
}

static struct mentry *echainadd(struct mentry *e, struct mentry *new) {
  new->next = e;
  return new;
}

static void extendtable(MTable *t) {
  int oldnslot = t->nslot;
  t->nslot = nslot_from(t->nentry);
  t->nentry = 0;
  struct mentry **old = t->e;
  t->e = calloc(1, sizeof(struct mentry *) * t->nslot);

  struct mentry *next;
  for(int i = 0; i < oldnslot; i++) {
    for(struct mentry *e = old[i]; e; e = next) {
      mtable_add(t, e->key, e->val);
      next = e->next;
      free(e);
    }
  }

  free(old);
}

void mtable_add(MTable *t, MxcValue key, MxcValue val) {
  if(++t->nentry > t->nslot)
    extendtable(t);

  uint32_t i = mval_hash32(key) % t->nslot;
  struct mentry *new = new_entry(key, val);
  if(t->e[i])
    t->e[i] = echainadd(t->e[i], new);
  else
    t->e[i] = new;
}

static void table_dealloc(MxcObject *a) {
  MTable *t = (MTable *)a;
  Mxc_free(t);
}

static void table_gc_mark(MxcObject *a) {
  if(OBJGCMARKED(a)) return;
  OBJGCMARK(a);
  MTable *t = (MTable *)a;
  for(int i = 0; i < t->nslot; i++) {
    for(struct mentry *e = t->e[i]; e; e = e->next) {
      mgc_mark(e->key);
      mgc_mark(e->val);
    }
  }
}

static void table_gc_guard(MxcObject *a) {
  OBJGCGUARD(a);
  MTable *t = (MTable *)a;
  for(int i = 0; i < t->nslot; i++) {
    for(struct mentry *e = t->e[i]; e; e = e->next) {
      mgc_guard(e->key);
      mgc_guard(e->val);
    }
  }
}

static void table_gc_unguard(MxcObject *a) {
  OBJGCUNGUARD(a);
  MTable *t = (MTable *)a;
  for(int i = 0; i < t->nslot; i++) {
    for(struct mentry *e = t->e[i]; e; e = e->next) {
      mgc_unguard(e->key);
      mgc_unguard(e->val);
    }
  }
}

static MxcValue table_tostring(MxcObject *a) {
  MxcValue res;
  MTable *t = (MTable *)a;
  GC_GUARD(t);

  if(t->nentry == 0) {
    res = new_string_static("{}", 2);
    GC_UNGUARD(t);
    return res;
  }

  res = new_string_static("{", 1);
  mgc_guard(res);

  int c = 0;
  for(int i = 0; i < t->nslot; i++) {
    for(struct mentry *e = t->e[i]; e; e = e->next) {
      if(c++ > 0)
        str_cstr_append(ostr(res), ", ", 2);
      MxcValue key_s = mval2str(e->key);
      MxcValue val_s = mval2str(e->val);
      str_append(ostr(res), ostr(key_s));
      str_cstr_append(ostr(res), "=>", 2);
      str_append(ostr(res), ostr(val_s));
    }
  }

  str_cstr_append(ostr(res), "}", 1);

  GC_UNGUARD(t);
  mgc_unguard(res);

  return res;
}

void table_set_default(MTable *t, MxcValue def) {
  t->default_val = def;
}

MxcValue tablegetitem(MxcIterable *self, MxcValue index) {
  MTable *t = (MTable *)self;
  uint32_t i = mval_hash32(index) % t->nslot;

  struct mentry *e = NULL;
  for(e = t->e[i]; e; e = e->next) {
    if(mval_eq(e->key, index))
      break;
  }

  if(e) {
    return e->val;
  }
  else if(check_value(t->default_val)) {
    return t->default_val;
  }
  else {
    mxc_raise(EXC_UNKNOWN_KEY, "unknown key: `%s`", mval2str(index));
    return mval_invalid;
  }
}

MxcValue tablesetitem(MxcIterable *self, MxcValue index, MxcValue a) {
  MTable *t = (MTable *)self;
  uint32_t i = mval_hash32(index) % t->nslot;

  struct mentry *e = NULL;
  for(e = t->e[i]; e; e = e->next) {
    if(mval_eq(e->key, index))
      break;
  }

  if(e)
    e->val = a;
  else
    mtable_add(t, index, a);

  return a;
}

struct mobj_system table_sys = {
  "table",
  NULL,
  table_tostring,
  table_dealloc,
  0,  // TODO
  table_gc_mark,
  table_gc_guard,
  table_gc_unguard,
  tablegetitem,
  tablesetitem,
  0,
  0,
  0,
  0,
};
